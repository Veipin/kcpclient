#ifndef __SOCKET_H__
#define __SOCKET_H__
////////////////////////////////////////////////////////////////////////////////
///
/// @file Socket.h
///
/// @brief Socket class declaration.
///
/// VSocket class is used to communicate with RAW, TCP and UDP socket.
///
////////////////////////////////////////////////////////////////////////////////

#include <string>                 // std::string
#include <sys/socket.h>           // inet_ntoa
#include <netinet/in.h>           // inet_ntoa, in_addr_t
#include <arpa/inet.h>            // inet_ntoa
#include <netdb.h>                // gethostbyname
#include <iostream>               // std::cout
#include <sstream>                // std::ostringstream
#include <map>                      
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <list>

#include "Type.hpp"
#ifdef USE_NDK_ENV
#include "jni/NdkEnv.h"
#endif


////////////////////////////////////////////////////////////////////////////////
///
/// @class SocketAddress
///
/// This class is used to convert address formats.
///
////////////////////////////////////////////////////////////////////////////////
class SocketAddress
{
public:

    SocketAddress();
    SocketAddress(const in_addr_t address, const in_port_t port_num);
    SocketAddress(const char* const address, const in_port_t port_num);
    SocketAddress(const SocketAddress& address);
    SocketAddress(const struct sockaddr_in& address);
    SocketAddress(const struct sockaddr& address);
    SocketAddress(const char* const addrAndPortString);
    SocketAddress(const std::string& addrAndPortString);
    virtual ~SocketAddress();

    void Clear();

    void SetIpAndPort(const std::string& addrAndPortString);
    void SetIpAndPort(const char* const addrAndPortString);
    std::string ToString() const;

    in_addr_t   GetIpAddress() const;
    std::string IpAddressToString() const;
    void SetIpAddress(const in_addr_t address);
    void SetIpAddress(const struct sockaddr_in& address);
    void SetIpAddress(const struct sockaddr& address);
    void SetIpAddress(const SocketAddress& address);
    void SetIpAddress(const std::string& address);
    void SetIpAddress(const char* const address);

    in_port_t GetPort() const;
    void SetPort(const in_port_t port_num);
    void SetPort(const SocketAddress& address);
    void SetPort(const struct sockaddr_in& address);
    void SetPort(const struct sockaddr& address);

    // let compiler do this one
    //SocketAddress &operator=( const SocketAddress& address );
    SocketAddress& operator=(const struct sockaddr_in& address);
    bool operator==(const SocketAddress& address) const;
    //bool operator==(const struct sockaddr_in& address);
    bool operator!=(const SocketAddress& address) const;
    //bool operator!=(const struct sockaddr_in& address);
    bool operator<(const SocketAddress& address) const;
    bool operator<(const struct sockaddr_in& address) const;
    bool operator<=(const SocketAddress& address) const;
    bool operator<=(const struct sockaddr_in& address) const;
    bool operator>(const SocketAddress& address) const;
    bool operator>(const struct sockaddr_in& address) const;
    bool operator>=(const SocketAddress& address) const;
    bool operator>=(const struct sockaddr_in& address) const;
    operator const struct sockaddr_in* () const;
    operator const struct sockaddr* () const;
    operator const struct sockaddr_in& () const;
    operator const struct sockaddr& () const;
    operator struct sockaddr_in* ();
    operator struct sockaddr* ();
    operator struct sockaddr_in& ();
    operator struct sockaddr& ();
    
    static SocketAddress GetLocalAddress();

private:

    in_addr_t ConvertStringToIpAddress(const char* const address);

    struct sockaddr_in m_sockAddr;
};


////////////////////////////////////////////////////////////////////////////////
///
/// @class VSocket
///
/// This class is a pure virtual class and used as base class.
///
////////////////////////////////////////////////////////////////////////////////
class VSocket
{
public:

    typedef std::list<in_port_t>    PortList;

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Constructor
    /// @param[in] addressFamily - address family. Default is AF_INET.
    /// @param[in] type - socket type. Default is UDP socket.
    /// @param[in] protocol - protocol type. Default is IP.
    ////////////////////////////////////////////////////////////////////////////
    VSocket(int GetAddressFamily = AF_INET, 
            int type = SOCK_DGRAM, 
            int protocol = IPPROTO_IP);

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Destructor
    ////////////////////////////////////////////////////////////////////////////
    virtual ~VSocket() = 0;

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Create socket
    /// @param[in] port - socket port. If zero, the port is assigned by system.
    /// @param[in] ipAddr - IP address. Default is any.
    /// @param[in] isReuseAddr - if reuse address. Default is true.
    /// @return socket fd (>=0) if successful, otherwise -1
    ////////////////////////////////////////////////////////////////////////////
    virtual int Create(in_port_t port = 0, in_addr_t ipAddr = INADDR_ANY, bool isReuseAddr = true);

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Create socket
    /// @param[in] sockAddr - socket address. 
    /// @param[in] isReuseAddr - if reuse address. Default is true.
    /// @return socket fd (>=0) if successful, otherwise -1
    ////////////////////////////////////////////////////////////////////////////
    virtual int Create(const SocketAddress& sockAddr, bool isReuseAddr = true);

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Create socket
    /// @param[in] portList - port list.
    /// @param[in] ipAddr - IP address. Default is any.
    /// @return socket fd (>=0) if successful, otherwise -1
    ////////////////////////////////////////////////////////////////////////////
    virtual int Create(const PortList& portList, const in_addr_t ipAddr = INADDR_ANY);

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Close socket
    /// @return none
    ////////////////////////////////////////////////////////////////////////////
    virtual void Close();

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Send data out of socket
    /// @param[in] data - data buffer to be sent
    /// @param[in] size - data size
    /// @param[in] to - the address where data is sent to.
    /// @return size of data sent if successful, otherwise -1
    ////////////////////////////////////////////////////////////////////////////
    virtual int Send(const void* data, int size, const SocketAddress& to);
    
    ////////////////////////////////////////////////////////////////////////////
    /// @brief Receive data from socket
    /// @param[in] data - data buffer
    /// @param[in] size - max data buffer size
    /// @param[in] from - the address where data is from
    /// @return size of data received if successful, otherwise -1
    ////////////////////////////////////////////////////////////////////////////
    virtual int Recv(void* data, int size, SocketAddress& from);

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Wait for input and output
    /// @param[in] waitMilliSec - wait time im milliseconds
    /// @param[inout] waitIn - if waiting for input.
    /// @param[inout] waitOut - if wanting for output.
    /// @return
    ///    >0: available for input(waitIn=true) or output(waitOut=true).
    ///    =0: timeout
    ///    <0: error
    ////////////////////////////////////////////////////////////////////////////
    virtual int Wait(int waitMilliSec, bool& waitIn, bool& waitOut);
    
    ////////////////////////////////////////////////////////////////////////////
    /// @brief Wait for input
    /// @param[in] waitMilliSec - wait time im milliseconds. Refer to WaitFlag.
    ///                           If WAIT_FOREVER, block until data arrive.
    ///                           If NO_WAIT, return immediately.
    ///                           Default is WAIT_FOREVER.
    /// @return true if input is available, otherwise false.
    ////////////////////////////////////////////////////////////////////////////
    virtual bool WaitInput(int waitMilliSec = WAIT_FOREVER);
    
    ////////////////////////////////////////////////////////////////////////////
    /// @brief Wait for output
    /// @param[in] waitMilliSec - wait time im milliseconds. Refer to WaitFlag.
    ///                           If WAIT_FOREVER, block until data arrive.
    ///                           If NO_WAIT, return immediately.
    ///                           Default is WAIT_FOREVER.
    /// @return true if output is available, otherwise false.
    ////////////////////////////////////////////////////////////////////////////
    virtual bool WaitOutput(int waitMilliSec = WAIT_FOREVER);

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Set if socket is blocked.
    /// @param[in] isBlock - if socket is blocked. 
    /// @return none
    ////////////////////////////////////////////////////////////////////////////
    virtual void SetBlock(bool isBlock);
    
    ////////////////////////////////////////////////////////////////////////////
    /// @brief Set recv buffer size
    /// @param[in] size - recv buffer size
    /// @return none
    ////////////////////////////////////////////////////////////////////////////
    virtual void SetRxBufSize(int size);

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Set send buffer size
    /// @param[in] size - send buffer size
    /// @return none
    ////////////////////////////////////////////////////////////////////////////
    virtual void SetTxBufSize(int size);

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Get address family
    /// @return address family
    ////////////////////////////////////////////////////////////////////////////
    inline int GetAddressFamily() const
    {
        return m_addressFamily;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Get socket type
    /// @return socket type
    ////////////////////////////////////////////////////////////////////////////
    inline int GetType() const
    {
        return m_type;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Get protocol type
    /// @return protocol type
    ////////////////////////////////////////////////////////////////////////////
    inline int GetProtocol() const
    {
        return m_protocol;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Get socket fd
    /// @return socket fd
    ////////////////////////////////////////////////////////////////////////////
    inline int GetFd() const
    {
        return m_sockFd;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Get socket port
    /// @return socket port
    ////////////////////////////////////////////////////////////////////////////
    inline in_port_t GetPort() const
    {
        return m_sockAddr.GetPort();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Get IP address
    /// @return IP address
    ////////////////////////////////////////////////////////////////////////////
    inline in_addr_t GetIpAddr() const
    {
        return m_sockAddr.GetIpAddress();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Get socket address(IP address and Port)
    /// @return socket address
    ////////////////////////////////////////////////////////////////////////////
    inline const SocketAddress& GetAddress() const
    {
        return m_sockAddr;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Get socket address(IP address and Port) string
    /// @return socket address string
    ////////////////////////////////////////////////////////////////////////////
    inline const std::string& GetAddressString() const
    {
        return m_addrString;
    }
    
    enum
    {
        /// Invalid socket fd value
        INVALID_FD = -1,
    };


protected:

    void GetLocalAddress();

    /// socket fd
    int m_sockFd;
    
private:

    /// Forbid copy constructor
    VSocket(const VSocket&);
    /// Forbid assignment operator
    VSocket& operator=(const VSocket&);

    /// address family
    int m_addressFamily;

    /// socket type
    int m_type;

    /// protocol type
    int m_protocol;

    /// socket address
    SocketAddress m_sockAddr;
    std::string m_addrString;
    
};

////////////////////////////////////////////////////////////////////////////////
///
/// @class RawSocket
///
/// This class uses RAW socket for communication.
///
////////////////////////////////////////////////////////////////////////////////
class RawSocket : public VSocket
{
public:

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Constructor
    /// @param[in] protocol - protocol type. Default is RAW.
    ////////////////////////////////////////////////////////////////////////////
    RawSocket(int protocol = IPPROTO_RAW):
    VSocket(AF_INET, SOCK_RAW, protocol)
    {
    }

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Destructor
    ////////////////////////////////////////////////////////////////////////////
    virtual ~RawSocket() {}

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Create socket
    /// @param[in] port - socket port. Unused.
    /// @param[in] ipAddr - IP address. Unused.
    /// @return socket fd (>=0) if successful, otherwise -1
    ////////////////////////////////////////////////////////////////////////////
    virtual int Create(in_port_t port = 0, in_addr_t ipAddr = INADDR_ANY);

};


////////////////////////////////////////////////////////////////////////////////
///
/// @class UdpSocket
///
/// This class uses UDP socket for communication.
///
////////////////////////////////////////////////////////////////////////////////
class UdpSocket : public VSocket
{
public:

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Constructor
    ////////////////////////////////////////////////////////////////////////////
    UdpSocket():
    VSocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)
    {
    }

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Destructor
    ////////////////////////////////////////////////////////////////////////////
    virtual ~UdpSocket() {}

};


////////////////////////////////////////////////////////////////////////////////
///
/// @class VTcpSocket
///
/// This class is a pure virtual class and used for base class.
///
////////////////////////////////////////////////////////////////////////////////
class VTcpSocket : public VSocket
{
public:

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Constructor
    ////////////////////////////////////////////////////////////////////////////
    VTcpSocket();

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Destructor
    ////////////////////////////////////////////////////////////////////////////
    virtual ~VTcpSocket() = 0;

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Close socket
    /// @return none
    ////////////////////////////////////////////////////////////////////////////
    virtual void Close();

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Send data out of socket
    /// @param[in] data - data buffer to be sent
    /// @param[in] size - data size
    /// @param[in] to - the address where data is sent to.
    /// @return size of data sent if successful, otherwise -1
    ////////////////////////////////////////////////////////////////////////////
    virtual int Send(const void* data, int size, const SocketAddress& to);
    
    ////////////////////////////////////////////////////////////////////////////
    /// @brief Receive data from socket
    /// @param[in] data - data buffer
    /// @param[in] size - max data buffer size
    /// @param[in] from - the address where data is from
    /// @return size of data received if successful, otherwise -1
    ////////////////////////////////////////////////////////////////////////////
    virtual int Recv(void* data, int size, SocketAddress& from);

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Send data out of socket
    /// @param[in] data - data buffer to be sent
    /// @param[in] size - data size
    /// @return size of data sent if successful, otherwise -1
    ////////////////////////////////////////////////////////////////////////////
    virtual int Send(const void* data, int size);
    
    ////////////////////////////////////////////////////////////////////////////
    /// @brief Receive data from socket
    /// @param[in] data - data buffer
    /// @param[in] size - max data buffer size
    /// @return size of data received if successful, otherwise -1
    ////////////////////////////////////////////////////////////////////////////
    virtual int Recv(void* data, int size);

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Check if socket is connected
    /// @return true if connected, otherwise false
    ////////////////////////////////////////////////////////////////////////////
    inline bool IsConnected() const
    {
        return m_isConnected;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Check if socket is server
    /// @return true if server, otherwise false
    ////////////////////////////////////////////////////////////////////////////
    inline bool IsSrv() const
    {
        return m_isSrv;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// @brief set if socket is server
    /// @return none
    ////////////////////////////////////////////////////////////////////////////
    inline void SetSrv(bool flag)
    {
        m_isSrv = flag;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Get remote address
    /// @return remote address
    ////////////////////////////////////////////////////////////////////////////
    inline const SocketAddress& GetRemoteAddress() const
    {
        return m_remoteAddr;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Get local address string
    /// @return local address string
    ////////////////////////////////////////////////////////////////////////////
    inline const std::string& GetLocalAddressString() const
    {
        return GetAddressString();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Get remote address string
    /// @return remote address string
    ////////////////////////////////////////////////////////////////////////////
    inline const std::string& GetRemoteAddressString() const
    {
        return m_remoteAddrString;
    }
    
protected:

    /// a flag if connected
    bool m_isConnected;

    ///a flag if server
    bool m_isSrv;

    /// remote address
    SocketAddress m_remoteAddr;
    std::string m_remoteAddrString;
};

////////////////////////////////////////////////////////////////////////////////
///
/// @class TcpClient
///
/// This class acts as TCP client for communication.
///
////////////////////////////////////////////////////////////////////////////////
class TcpClient : public VTcpSocket
{
public:

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Constructor
    ////////////////////////////////////////////////////////////////////////////
    TcpClient();

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Constructor
    /// @param[in] fd - socket fd already opened
    /// @param[in] remoteAddress - remote address
    ////////////////////////////////////////////////////////////////////////////
    TcpClient(int fd, SocketAddress& remoteAddress);
    
    ////////////////////////////////////////////////////////////////////////////
    /// @brief Destructor
    ////////////////////////////////////////////////////////////////////////////
    virtual ~TcpClient();

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Set new TCP link
    /// @param[in] fd - socket fd already opened
    /// @param[in] remoteAddress - remote address
    /// @return none
    ////////////////////////////////////////////////////////////////////////////
    void Set(int fd, SocketAddress& remoteAddress);

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Connect to remote TCP socket
    /// @param[in] remoteAddress - remote address
    /// @return 0 if successful, otherwise -1
    ////////////////////////////////////////////////////////////////////////////
    virtual int Connect(const SocketAddress& remoteAddress);

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Connect to remote TCP socket
    /// @param[in] remoteAddress - remote address
    /// @param[in] waitMilliSeconds - wait time in milliseconds
    /// @return 0 if successful, otherwise -1
    ////////////////////////////////////////////////////////////////////////////
    virtual int Connect(const SocketAddress& remoteAddress, uint32_t waitMilliSeconds);
    int GetTryTimes(){return uAcceptTimes;}

public:
        int uAcceptTimes;
private:

    
};

////////////////////////////////////////////////////////////////////////////////
///
/// @class TcpClient
///
/// This class acts as TCP server for communication.
///
////////////////////////////////////////////////////////////////////////////////
class TcpServer : public VTcpSocket
{
public:

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Constructor
    ////////////////////////////////////////////////////////////////////////////
    TcpServer();

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Destructor
    ////////////////////////////////////////////////////////////////////////////
    virtual ~TcpServer();

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Listen for socket connections
    /// @param[in] waitNum - max number of incoming connections. Default is 1.
    /// @return 0 if successful, otherwise -1
    ////////////////////////////////////////////////////////////////////////////
    virtual int Listen(int waitNum = 1);

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Accept a connection on a socket
    /// @param[out] tcpClient - TCP client
    /// @return 0 if successful, otherwise -1
    ////////////////////////////////////////////////////////////////////////////
    virtual int Accept(TcpClient& tcpClient);
    
    
};


template <class T>
class TUdpMsg
{

public:

    typedef TUdpMsg<T> UdpMsg;
    typedef void (T::*MsgHandler)(const void* msg, uint32_t msgLen, UdpMsg& udpMsg);
    typedef void (T::*UnknownMsgHandler)(uint32_t type, const void* msg, uint32_t msgLen, UdpMsg& udpMsg);

    struct MsgHandlerTable
    {
        uint32_t type;
        MsgHandler handler;
    };

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Constructor
    ////////////////////////////////////////////////////////////////////////////
    TUdpMsg(T* object):
    m_object(object),
    m_msgBufLen(0),
    m_msgBuf(NULL),
    m_rxMsg(NULL),
    m_txMsg(NULL),
    m_unknownMsgHandler(NULL)
    {
        SetMsgBufSize(MSG_BUF_LEN);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Destructor
    ////////////////////////////////////////////////////////////////////////////
    virtual ~TUdpMsg()
    {
        m_udp.Close();
        m_msgHandler.clear();

        if (m_msgBuf != NULL)
        {
            delete[] m_msgBuf;
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Set msg buffer
    /// @param[in] buf - buffer address
    /// @param[in] bufLen - buffer length
    /// @return none
    ////////////////////////////////////////////////////////////////////////////
    inline void SetMsgBufSize(uint32_t bufLen)
    {
        size_t msgBlockSize = bufLen+MSG_HEAD_LEN;

        if (m_msgBufLen == msgBlockSize)
        {
            return;
        }
        
        if (m_msgBuf != NULL)
        {
            delete[] m_msgBuf;
            m_msgBuf = NULL;
            m_rxMsg = NULL;
            m_txMsg = NULL;
            m_msgBufLen = 0;
        }
        
        m_msgBuf = (char*) new(std::nothrow) char[msgBlockSize<<1];
        if (NULL == m_msgBuf)
        {
            return;
        }

        m_rxMsg = (Msg*)m_msgBuf;
        m_txMsg = (Msg*)(m_msgBuf + msgBlockSize);
        m_msgBufLen = msgBlockSize;
        
    }

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Create socket
    /// @param[in] port - socket port. If zero, the port is assigned by system.
    /// @param[in] ipAddr - IP address. Default is any.
    /// @return socket fd (>=0) if successful, otherwise -1
    ////////////////////////////////////////////////////////////////////////////
    int CreateSocket(in_port_t port = 0, in_addr_t ipAddr = INADDR_ANY)
    {
        int rc = m_udp.Create(port, ipAddr);
        //m_udp.SetBlock(false);
        
        m_myAddr = m_udp.GetAddress();

        return rc;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Close socket
    /// @return none
    ////////////////////////////////////////////////////////////////////////////
    void CloseSocket()
    {
        m_udp.Close();
    }
    
    ////////////////////////////////////////////////////////////////////////////
    /// @brief Set msg handler table
    /// @param[in] table - msg handler table. It must end with {0,0}.
    /// @return none
    ////////////////////////////////////////////////////////////////////////////
    void SetMsgHandlerTable(MsgHandlerTable* table)
    {
        m_msgHandler.clear();

        MsgHandlerTable* t = table;
        while (true)
        {
            if (0 == t->handler)
            {
                break;
            }

            m_msgHandler[t->type] = t->handler;
            ++t;
        }
    }
    
    ////////////////////////////////////////////////////////////////////////////
    /// @brief Set unknown msg handler table
    /// @param[in] handler - unknown msg handler
    /// @return old handler
    ////////////////////////////////////////////////////////////////////////////
    inline UnknownMsgHandler SetUnknownMsgHandler(UnknownMsgHandler handler)
    {
        UnknownMsgHandler old = m_unknownMsgHandler;
        m_unknownMsgHandler = handler;
        return old;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Get socket fd
    /// @return socket fd
    ////////////////////////////////////////////////////////////////////////////
    inline int GetFd() const
    {
        return m_udp.GetFd();
    }

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Get local address
    /// @return local address
    ////////////////////////////////////////////////////////////////////////////
    inline const SocketAddress& GetLocalAddr() const
    {
        return m_myAddr;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Get from address
    /// @return from address
    ////////////////////////////////////////////////////////////////////////////
    inline const SocketAddress& GetFromAddr() const
    {
        return m_fromAddr;
    }
    
    ////////////////////////////////////////////////////////////////////////////
    /// @brief Get socket fd
    /// @return socket fd
    ////////////////////////////////////////////////////////////////////////////
    void SocketHandler(int fd, int event)
    {
        (void)fd;       // unused
        (void)event;    // unused
        
        int rc = RecvMsg();
        if (rc > 0)
        {
            ProcMsg();
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Recv msg from socket
    /// @return msg length
    ////////////////////////////////////////////////////////////////////////////
    int RecvMsg()
    {
        if ((NULL == m_rxMsg) || (0 == m_msgBufLen))
        {
            return -1;
        }
        
        int rc = m_udp.Recv(m_rxMsg, m_msgBufLen, m_fromAddr);
    
        return rc;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Send msg
    /// @param[in] type - msg type
    /// @param[in] body - msg body
    /// @param[in] bodySize - msg body size
    /// @param[in] to - address where msg sent to
    /// @return 0 if successful, otherwise -1
    ////////////////////////////////////////////////////////////////////////////
    int SendMsg(int type, const void* body, int bodySize, const SocketAddress& to)
    {
        if ((NULL == m_txMsg) || (0 == m_msgBufLen))
        {
            return -1;
        }

        m_txMsg->head.type = htonl(type);
        m_txMsg->head.bodyLen = 0;

        size_t sendSize = MSG_HEAD_LEN;
        if ((body != NULL) && (bodySize > 0))
        {
            sendSize += bodySize;
            m_txMsg->head.bodyLen = htonl(bodySize);

            memcpy(m_txMsg->body, body, bodySize);
        }
        
        return m_udp.Send(m_txMsg, sendSize, to);
    }

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Reply msg
    /// @param[in] type - msg type
    /// @param[in] body - msg body
    /// @param[in] bodySize - msg body size
    /// @return none
    ////////////////////////////////////////////////////////////////////////////
    void ReplyMsg(int type, const void* body, int bodySize)
    {
        SendMsg(type, body, bodySize, m_fromAddr);
    }

private:

    struct Head
    {
        uint32_t type;
        uint32_t bodyLen;
    };

    enum
    {
        MSG_HEAD_LEN = sizeof(Head),
        MSG_BUF_LEN = 2048,
    };
    
    struct Msg
    {
        Head head;
        char body[1];
    };

    typedef std::map<uint32_t, MsgHandler> MsgHandlerMap;
    typedef typename MsgHandlerMap::iterator MsgHandlerMapIt;

    ////////////////////////////////////////////////////////////////////////////
    /// @brief Process msg
    /// @return none
    ////////////////////////////////////////////////////////////////////////////  
    void ProcMsg()
    {
        uint32_t msgType = ntohl(m_rxMsg->head.type);
        uint32_t msgBodyLen = ntohl(m_rxMsg->head.bodyLen);

        MsgHandlerMapIt it = m_msgHandler.begin();

        for (; it != m_msgHandler.end(); ++it)
        {
            uint32_t type = it->first;
            MsgHandler handler = it->second;

            if (msgType == type)
            {
                (m_object->*handler)(m_rxMsg->body, msgBodyLen, *this);
                return;
            }
        }

        if (m_unknownMsgHandler != 0)
        {
            (m_object->*m_unknownMsgHandler)(msgType, m_rxMsg->body, msgBodyLen, *this);
        }
    }

    /// object
    T* m_object;

    /// msg buffer length
    uint32_t m_msgBufLen;
    
    /// msg buffer
    char* m_msgBuf;
    Msg* m_rxMsg;
    Msg* m_txMsg;

    /// udp socket
    UdpSocket m_udp;

    /// my address
    SocketAddress m_myAddr;

    /// address of received packet
    SocketAddress m_fromAddr;

    /// msg handler map
    MsgHandlerMap m_msgHandler;

    /// unknown msg handler
    UnknownMsgHandler m_unknownMsgHandler;
};

struct IfIpAddr
{
    std::string ifName;
    std::string ipAddr;
};
typedef std::list<IfIpAddr> IfIpAddrList;

int GetLocalIpAddrList(IfIpAddrList& list);



#endif // __SOCKET_H__


