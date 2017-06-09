//=====================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdint.h>

#include "kcpclient.h"
#include "LibLog.h"
#include "LibTime.h"

static int gRun = 1;
UdpSocket sock;

    
/*F InitLogInfo()
初始化日志*/
void InitLogInfo()
{
    InitPublic((char *)PROJ_DIR);
    InitLog(APP_NAME,LOG_DAYLIY);
}

/*F Signal_hander()
信号处理函数 */
void Signal_hander(int siga)
{
    gRun = 0;
}
void Signal_Pipe(int siga)
{
    //PrintSignal(siga);
}

void InitSignalProcess()
{
    signal(SIGINT, (void(*)(int))Signal_hander);//SIGINT 2 (中断) 当用户按下时,通知前台进程组终止进程
    signal(SIGQUIT,(void(*)(int))Signal_hander);//SIGQUIT 3 (退出) 用户按下或时通知进程，使进程终止
    signal(SIGTERM,(void(*)(int))Signal_hander);//SIGTERM 15 (软中断) 使用不带参数的kill命令时终止进程
    signal(SIGABRT,(void(*)(int))Signal_hander);//SIGABRT 6 (异常中止) 调用abort函数生成的信号
    signal(SIGPWR, (void(*)(int))Signal_hander);//SIGPWR 30 电源失效/重启动

    sigignore(SIGHUP); //SIGHUP 1 (挂起) 当运行进程的用户注销时通知该进程，使进程终止
    signal(SIGPIPE,(void(*)(int))Signal_Pipe);  //SIGPIPE 13 写至无读进程的管道, 或者Socket通信SOCT_STREAM的读进程已经终止，而再写入
}


int udp_output(const char *buf, int len, ikcpcb *kcp, void *user)
{
	SocketAddress *pto = (SocketAddress *)user;
	int ret = sock.Send(buf, len, *pto);
	AppLog(LOG_BASE, "udp_output send len %d\n", len);
	return ret;
}

int main(int argc,char *argv[])
{
    InitLogInfo();
    InitSignalProcess();
    
    SetAppLogPrint(true);
    SetAppLogLogGroup(false);
    AppLog(LOG_BASE, "kcpclient start\n");

    if (argc != 4)
    {
        AppLog(LOG_BASE, "param err, please input: ./kcpclient localport desIp:port sendTimes\n");
        return 0;
    }

    int localport = (int)atoi(argv[1]);

    sock.Create(localport);
    
    SocketAddress to;
    to.SetIpAndPort(argv[2]);
    SocketAddress from;
        
    char body[128];
    int  dataLen = sprintf(body, "%d hello world-0", localport); 
    char buf[128];
    int recvflag, recvDataLen;
    int sendTimes = (int)atoi(argv[3]);
    
    int  i, lostflag, index = 0;
    char tmpbuf[128] = {0};

    ikcpcb *kcp = ikcp_create(0x01, (void*)&to);
    kcp->output = udp_output;
    //ikcp_wndsize(kcp, 32, 32);
    ikcp_nodelay(kcp, 1, 10, 2, 1); // 快速模式 0-RTO100ms  10ms-执行间隔  2_快速重传  1-关闭流控
    //ikcp_nodelay(kcp, 0, 10, 0 ,0); // 默认模式
    
    uint64_t timeNow = GetCurrTimeAsLong(); // ms
    ikcp_update(kcp,  timeNow);
    ikcp_send(kcp, body, dataLen+1);

    uint64_t lastSendtime = timeNow;

    while (gRun)
    {
    	timeNow = GetCurrTimeAsLong();
		ikcp_update(kcp,  timeNow);

	
        recvflag = sock.WaitInput(10);
        if (recvflag)
        {
            recvDataLen = sock.Recv(buf, 128, from);
            AppLog(LOG_BASE, "--- sock.Recv len:%d  from:%s  timeNow:%05lu\n", recvDataLen, from.ToString().data(), timeNow%100000);
            if (recvDataLen > 0)
            {
                if (from != to)
                {
                    to = from;
                    AppLog(LOG_BASE, "chang dst addr to %s\n",to.ToString().data());
                }

                // 模拟网络丢包
                lostflag = 0;
                for (i=0; i<recvDataLen-2; i++)
                {
                    if (buf[i]==0x39 && buf[i+1]==0x38 && buf[i+2]==0x00)
                        break;
                }
                if (i<recvDataLen-2)
                {
                    memcpy(tmpbuf, buf+i, 3);
                    //lostflag = 1;
                }
                ++index;
                if (index%4 == 0)
                    lostflag = 1;

                if (lostflag == 0)
                    ikcp_input(kcp, buf, recvDataLen);
                else
                    AppLog(LOG_BASE, "*** lost packet:%d   buf:%s  timeNow:%05lu\n", index, tmpbuf, timeNow%100000);
            }
        }

        timeNow = GetCurrTimeAsLong();
        recvDataLen = ikcp_recv(kcp, buf, 128);
        if (recvDataLen > 0)
        {
            AppLog(LOG_BASE, "=== ikcp_recv len:%d  data:[%s]  timeNow:%05lu\n", recvDataLen, buf, timeNow%100000);
        }

        if ((index>0) && (timeNow-lastSendtime) >= 20)
        {
            lastSendtime = timeNow;
            
            if ((--sendTimes) > 0)
            {
                dataLen = sprintf(body, "%d hello world-%d", localport, sendTimes);          
                ikcp_send(kcp, body, dataLen+1);
                AppLog(LOG_BASE, "&&& ikcp_send  len:%d  sendTimes:%d\n", dataLen, sendTimes);
            }
        }
        
        //usleep(5*1000); 
    }

    ikcp_release(kcp);
    sock.Close();
    
    AppLog(LOG_BASE, "kcpclient over\n");
    return 0;
}













