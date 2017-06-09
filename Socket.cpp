////////////////////////////////////////////////////////////////////////////////
///
/// @file VSocket.cpp
///
/// @brief VSocket class definition.
///
/// VSocket class is used to communicate with RAW, TCP and UDP socket.
///
////////////////////////////////////////////////////////////////////////////////

#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/if.h>

#include "Socket.h"
#include "LibLog.h"


//------------------------------------------------------------------------
// default constructor
//------------------------------------------------------------------------
SocketAddress::SocketAddress()
{
    Clear();
}

//------------------------------------------------------------------------
// constructor where ip address and port number are supplied
//------------------------------------------------------------------------
SocketAddress::SocketAddress(const in_addr_t address, const in_port_t port_num)
{
    Clear();
    SetIpAddress(address);
    SetPort(port_num);
}

//------------------------------------------------------------------------
// constructor where ip address string and port number are supplied
//------------------------------------------------------------------------
SocketAddress::SocketAddress(const char* const address, const in_port_t port_num)
{
    Clear();
    SetIpAddress(address);
    SetPort(port_num);
}

//------------------------------------------------------------------------
// constructor where AAA.BBB.CCC.DDD:PPP are supplied
//------------------------------------------------------------------------
SocketAddress::SocketAddress(const char* const addrAndPortString)
{
    Clear();
    SetIpAndPort(addrAndPortString);
}

//------------------------------------------------------------------------
// constructor where AAA.BBB.CCC.DDD:PPP are supplied
//------------------------------------------------------------------------
SocketAddress::SocketAddress(const std::string& addrAndPortString)
{
    Clear();
    SetIpAndPort(addrAndPortString);
}

//------------------------------------------------------------------------
// copy constructor
//------------------------------------------------------------------------
SocketAddress::SocketAddress(const SocketAddress& address)
{
    Clear();
    m_sockAddr = *((const struct sockaddr_in *const)address);
}

//------------------------------------------------------------------------
// constructor where sockaddr_in struct is supplied
//------------------------------------------------------------------------
SocketAddress::SocketAddress(const struct sockaddr_in& address)
{
    Clear();
    m_sockAddr = address;
}

//------------------------------------------------------------------------
// constructor where sockaddr_in struct is supplied
//------------------------------------------------------------------------
SocketAddress::SocketAddress(const struct sockaddr& address)
{
    Clear();
    m_sockAddr = ((const struct sockaddr_in)*((sockaddr_in *)& address));
}


//------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------
SocketAddress::~SocketAddress()
{
}

//------------------------------------------------------------------------
// clear
//------------------------------------------------------------------------
void SocketAddress::Clear()
{
    memset(&m_sockAddr, 0, sizeof(m_sockAddr));
    m_sockAddr.sin_family = AF_INET;
}


//------------------------------------------------------------------------
// methods to set address where a string as AAA.BBB.CCC.DDD:PPP are supplied
//------------------------------------------------------------------------
void SocketAddress::SetIpAndPort(const std::string& addrAndPortString)
{
    SetIpAndPort(addrAndPortString.data());
}

//------------------------------------------------------------------------
// methods to set address where a string as AAA.BBB.CCC.DDD:PPP are supplied
//------------------------------------------------------------------------
void SocketAddress::SetIpAndPort(const char* const addrAndPortString)
{
    char* address = strdup(addrAndPortString);
    if (NULL == address)
    {
        return;
    }

    uint16_t portNum = 0;
    
    char* p = strchr(address, ':');
    if (NULL == p)
    {
        portNum = atoi(address);
        
        SetIpAddress(INADDR_LOOPBACK);
        SetPort(portNum);
    }
    else
    {
        *p++ = '\0';
        portNum = atoi(p);

        SetIpAddress(address);
        SetPort(portNum);
    }
 
    free(address);
}

//------------------------------------------------------------------------
// methods to show the ipaddress and port
//------------------------------------------------------------------------
std::string SocketAddress::ToString() const
{
    std::ostringstream oss;
    oss << IpAddressToString() << ":" << GetPort();
    
    return oss.str();
}

//------------------------------------------------------------------------
// methods to return the ip address
//------------------------------------------------------------------------
in_addr_t SocketAddress::GetIpAddress() const
{
    return ntohl(m_sockAddr.sin_addr.s_addr);
}

std::string SocketAddress::IpAddressToString() const
{
    return inet_ntoa(m_sockAddr.sin_addr);
}

//------------------------------------------------------------------------
// methods to set the ip address
//------------------------------------------------------------------------
void SocketAddress::SetIpAddress(const in_addr_t address)
{
    m_sockAddr.sin_addr.s_addr = htonl(address);
}

void SocketAddress::SetIpAddress(const SocketAddress& address)
{
    m_sockAddr.sin_addr.s_addr = ((const struct sockaddr_in *const)address)->sin_addr.s_addr;
}

void SocketAddress::SetIpAddress(const struct sockaddr_in& address)
{
    m_sockAddr.sin_addr.s_addr = address.sin_addr.s_addr;
}

void SocketAddress::SetIpAddress(const struct sockaddr& address)
{
    m_sockAddr.sin_addr.s_addr = ((const struct sockaddr_in) * ((sockaddr_in *)& address)).sin_addr.s_addr;
}

void SocketAddress::SetIpAddress(const std::string& address)
{
    SetIpAddress(address.data());
}

void SocketAddress::SetIpAddress(const char* const address)
{
    m_sockAddr.sin_addr.s_addr = ConvertStringToIpAddress(address);
}

//------------------------------------------------------------------------
// method to get the socket number
//------------------------------------------------------------------------
in_port_t SocketAddress::GetPort() const
{
    return ntohs(m_sockAddr.sin_port);
}

//------------------------------------------------------------------------
// methods to set the socket number
//------------------------------------------------------------------------
void SocketAddress::SetPort(const in_port_t port_num)
{
    m_sockAddr.sin_port = htons(port_num);
}

void SocketAddress::SetPort(const SocketAddress& address)
{
    m_sockAddr.sin_port = ((const struct sockaddr_in *const)address)->sin_port;
}

void SocketAddress::SetPort(const struct sockaddr_in& address)
{
    m_sockAddr.sin_port = address.sin_port;
}

void SocketAddress::SetPort(const struct sockaddr& address)
{
    m_sockAddr.sin_port = ((const struct sockaddr_in) * ((sockaddr_in *)& address)).sin_port;
}

//------------------------------------------------------------------------
// assignment operator - for sockaddr_in struct
//------------------------------------------------------------------------
SocketAddress& SocketAddress::operator=(const struct sockaddr_in& address)
{
    if (&address != &m_sockAddr)
    {
        m_sockAddr = address;
    }
    return *this;
}

//------------------------------------------------------------------------
// equality operator - for SocketAddress
//------------------------------------------------------------------------
bool SocketAddress::operator==(const SocketAddress& address) const
{
    return((m_sockAddr.sin_addr.s_addr == address.m_sockAddr.sin_addr.s_addr) &&
        (m_sockAddr.sin_port == address.m_sockAddr.sin_port));
}

//------------------------------------------------------------------------
// inequality operator - for SocketAddress
//------------------------------------------------------------------------
bool SocketAddress::operator!=(const SocketAddress& address) const
{
    return(!(*this == address));
}

//------------------------------------------------------------------------
// less than operator - for SocketAddress
//------------------------------------------------------------------------
bool SocketAddress::operator<(const SocketAddress& address) const
{
    return((m_sockAddr.sin_addr.s_addr < address.m_sockAddr.sin_addr.s_addr) ||
        ((m_sockAddr.sin_addr.s_addr == address.m_sockAddr.sin_addr.s_addr) &&
            (m_sockAddr.sin_port < address.m_sockAddr.sin_port)));
}

//------------------------------------------------------------------------
// less than operator - for SocketAddress
//------------------------------------------------------------------------
bool SocketAddress::operator<(const struct sockaddr_in& address) const
{
    return((m_sockAddr.sin_addr.s_addr < address.sin_addr.s_addr) ||
        ((m_sockAddr.sin_addr.s_addr == address.sin_addr.s_addr) &&
            (m_sockAddr.sin_port < address.sin_port)));
}

//------------------------------------------------------------------------
// less than or equal operator - for SocketAddress
//------------------------------------------------------------------------
bool SocketAddress::operator<=(const SocketAddress& address) const
{
    return(!(*this > address));
}

//------------------------------------------------------------------------
// less than or equal operator - for SocketAddress
//------------------------------------------------------------------------
bool SocketAddress::operator<=(const struct sockaddr_in& address)
const
{
    return(!(*this > address));
}

//------------------------------------------------------------------------
// greater than operator - for SocketAddress
//------------------------------------------------------------------------
bool SocketAddress::operator>(const SocketAddress& address) const
{
    return((m_sockAddr.sin_addr.s_addr > address.m_sockAddr.sin_addr.s_addr) ||
        ((m_sockAddr.sin_addr.s_addr == address.m_sockAddr.sin_addr.s_addr) &&
            (m_sockAddr.sin_port > address.m_sockAddr.sin_port)));
}

//------------------------------------------------------------------------
// greater than operator - for SocketAddress
//------------------------------------------------------------------------
bool SocketAddress::operator>(const struct sockaddr_in& address) const
{
    return((m_sockAddr.sin_addr.s_addr > address.sin_addr.s_addr) ||
        ((m_sockAddr.sin_addr.s_addr == address.sin_addr.s_addr) &&
            (m_sockAddr.sin_port > address.sin_port)));
}

//------------------------------------------------------------------------
// greater than or equal operator - for SocketAddress
//------------------------------------------------------------------------
bool SocketAddress::operator>=(const SocketAddress& address) const
{
    return(!(*this < address));
}

//------------------------------------------------------------------------
// greater than or equal - for SocketAddress
//------------------------------------------------------------------------
bool SocketAddress::operator>=(const struct sockaddr_in& address) const
{
    return(!(*this < address));
}

//------------------------------------------------------------------------
// casting operators to extract the sockaddr_in struct
//------------------------------------------------------------------------
SocketAddress::operator const struct sockaddr_in* () const
{
    return &m_sockAddr;
}

SocketAddress::operator struct sockaddr_in* ()
{
    return &m_sockAddr;
}

SocketAddress::operator const struct sockaddr_in& () const
{
    return m_sockAddr;
}

SocketAddress::operator struct sockaddr_in& ()
{
    return m_sockAddr;
}

//------------------------------------------------------------------------
// casting operators to extract the sockaddr struct
//------------------------------------------------------------------------
SocketAddress::operator const struct sockaddr * () const
{
    return (struct sockaddr *)&m_sockAddr;
}

SocketAddress::operator struct sockaddr * ()
{
    return (struct sockaddr *)&m_sockAddr;
}

SocketAddress::operator const struct sockaddr & () const
{
    return *((struct sockaddr *)&m_sockAddr);
}

SocketAddress::operator struct sockaddr & ()
{
    return *((struct sockaddr *)&m_sockAddr);
}

SocketAddress SocketAddress::GetLocalAddress()
{
    SocketAddress addr;
    addr.SetIpAddress((in_addr_t)-1);
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0)
    {
        addr.SetIpAddress(hostname);
    }
    return addr;
}

in_addr_t SocketAddress::ConvertStringToIpAddress(const char* const address)
{
    in_addr_t returnIpAddress;
    if (isdigit(address[0]))
    {
        returnIpAddress = inet_addr(address);
    }
    else
    {
        struct hostent *hostEntry = gethostbyname(address);
        if (hostEntry)
        {
            memcpy(&returnIpAddress, hostEntry->h_addr, sizeof(returnIpAddress));
        }
        else
        {
            returnIpAddress = (in_addr_t)-1;
        }
        // as suggested by man page,
        // still does not satisfy purify though (it still thinks there is an
        // fd open when the program quits)
        endhostent();
    }
    return returnIpAddress;
}


//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
VSocket::VSocket(int addressFamily, int type, int protocol):
    m_sockFd(INVALID_FD),
    m_addressFamily(addressFamily),
    m_type(type),
    m_protocol(protocol)
{
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
VSocket::~VSocket()
{
    Close();
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
int VSocket::Create(const in_port_t port, const in_addr_t ipAddr, bool isReuseAddr)
{
    Close();
    
    struct sockaddr_in address;

    // set address
    memset(&address, 0, sizeof(address));
    address.sin_family      = m_addressFamily;
    address.sin_addr.s_addr = htonl(ipAddr);
    address.sin_port        = htons(port);

    m_sockAddr = address;
    m_addrString = m_sockAddr.ToString();

    // create socket
    int fd = ::socket(m_addressFamily, m_type, m_protocol);
    if (INVALID_FD == fd)
    {
        PERROR("Failed to create socket");
        return INVALID_FD;
    }

    if (isReuseAddr)
    {
        // Avoid the annoying "Address already in use" bind error code
        int yes = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
        {
            PERROR("Failed to reuse socket %d", fd);
            ::close(fd);
            return INVALID_FD;
        }
    }

    // bind socket
    if (::bind(fd, (struct sockaddr *)&address, sizeof(address)) == -1)
    {
        PERROR("Failed to bind socket %d on %s", fd, m_sockAddr.ToString().data());
        ::close(fd);
        return INVALID_FD;
    }

    m_sockFd = fd;

    // get socket address
    if (0 == port)
    {
        GetLocalAddress();
    }
    
    return m_sockFd;
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
int VSocket::Create(const SocketAddress& sockAddr, bool isReuseAddr)
{
    return Create(sockAddr.GetPort(), sockAddr.GetIpAddress(), isReuseAddr);
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
int VSocket::Create(const PortList& portList, const in_addr_t ipAddr)
{
    Close();
    
    struct sockaddr_in address;

    // set address
    memset(&address, 0, sizeof(address));
    address.sin_family      = m_addressFamily;
    address.sin_addr.s_addr = htonl(ipAddr);

    // create socket
    int fd = ::socket(m_addressFamily, m_type, m_protocol);
    if (INVALID_FD == fd)
    {
        PERROR("Failed to create socket");
        return INVALID_FD;
    }

    int port = -1;
    PortList::const_iterator it = portList.begin();
    PortList::const_iterator end = portList.end();

    for (; it != end; ++it)
    {
        in_port_t p = *it;
        address.sin_port = htons(p);
        
        // bind socket
        if (0 == ::bind(fd, (struct sockaddr *)&address, sizeof(address)))
        {
            port = p;
            break;
        }
    }
    
    if (-1 == port)
    {
        PERROR("Failed to bind socket %d on ports", fd);
        ::close(fd);
        return INVALID_FD;
    }

    m_sockFd = fd;
    
    // get socket address
    if (0 == port)
    {
        GetLocalAddress();
    }
    else
    {
        m_sockAddr = address;
        m_addrString = m_sockAddr.ToString();
    }
    
    return m_sockFd;
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
void VSocket::Close()
{
    if (m_sockFd != INVALID_FD)
    {
        ::close(m_sockFd);
        m_sockFd = INVALID_FD;
    }
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
int VSocket::Send(const void* data, int size, const SocketAddress& to)
{
    if ((NULL == data) || (size <= 0))
    {
        return -1;
    }
    
    if (INVALID_FD == m_sockFd)
    {
        Create();
    }

    if (INVALID_FD == m_sockFd)
    {
        return -1;
    }

    int rc = 0;
    do
    {
        rc = ::sendto(m_sockFd, data, size, 0, to, sizeof(struct sockaddr_in));
    }
    while ((rc < 0) && (EINTR == errno));
    
    return rc;
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
int VSocket::Recv(void* data, int size, SocketAddress& from)
{
    if ((NULL == data) || (size <= 0))
    {
        return -1;
    }
    
    if (INVALID_FD == m_sockFd)
    {
        Create();
    }

    if (INVALID_FD == m_sockFd)
    {
        return -1;
    }

    socklen_t len = sizeof(struct sockaddr_in);
    int rc = 0;
    do
    {
        rc = ::recvfrom(m_sockFd, data, size, 0, from, &len);
    }
    while ((rc < 0) && (EINTR == errno));

    if ((rc < 0) && (EAGAIN == errno))
    {
        return 0;
    }

    return rc;
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
int VSocket::Wait(int waitMilliSec, bool& waitIn, bool& waitOut)
{
    if (INVALID_FD == m_sockFd)
    {
        Create();
    }

    if (INVALID_FD == m_sockFd)
    {
        return -1;
    }
        
    struct pollfd fd[1];

    fd[0].fd = m_sockFd;
    fd[0].events = 0;
    
    if (waitIn)
    {
        fd[0].events = POLLIN;
    }
    if (waitOut)
    {
        fd[0].events = POLLOUT;
    }

    waitIn = false;
    waitOut = false;

    int rc = 0;
    do
    {
        rc = poll(fd, 1, waitMilliSec);
    } 
    while ((rc < 0) && (EINTR == errno));

    if (rc > 0)
    {
        int count = 0;
        if (fd[0].revents & POLLIN)
        {
            ++count;
            waitIn = true;
        }
        if (fd[0].revents & POLLOUT)
        {
            ++count;
            waitOut= true;
        }

        return count;
    }
    else if (0 == rc)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
bool VSocket::WaitInput(int waitMilliSec)
{
    bool waitIn = true;
    bool waitOut = false;

    int rc = Wait(waitMilliSec, waitIn, waitOut);
    if (rc <= 0)
    {
        return false;
    }

    return waitIn;
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
bool VSocket::WaitOutput(int waitMilliSec)
{
    bool waitIn = false;
    bool waitOut = true;

    int rc = Wait(waitMilliSec, waitIn, waitOut);
    if (rc <= 0)
    {
        return false;
    }

    return waitOut;
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
void VSocket::SetBlock(bool isBlock)
{
    int sockFlag = fcntl(m_sockFd, F_GETFL, 0);
    if (-1 == sockFlag)
    {
        return;
    }

    if (isBlock)
    {
        sockFlag &= ~O_NONBLOCK;
    }
    else
    {
        sockFlag |= O_NONBLOCK;
    }
    
    fcntl(m_sockFd, F_SETFL, sockFlag);
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
void VSocket::SetRxBufSize(int size)
{
    setsockopt(m_sockFd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
void VSocket::SetTxBufSize(int size)
{
    setsockopt(m_sockFd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Get socket address
/// @return none
////////////////////////////////////////////////////////////////////////////////
void VSocket::GetLocalAddress()
{
    if (INVALID_FD == m_sockFd)
    {
        return;
    }
    
    struct sockaddr name;
    socklen_t addrSize = sizeof(name);
    
    if (::getsockname(m_sockFd, &name, &addrSize) == -1)
    {
        PERROR("Failed to get name of socket %d", m_sockFd);
    }

    m_sockAddr = name;
    m_addrString = m_sockAddr.ToString();
    
    return;
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
int RawSocket::Create(const in_port_t port, const in_addr_t ipAddr)
{
    (void)port; // unused
    (void)ipAddr; // unused
    
    Close();
    
    // create socket
    int fd = ::socket(GetAddressFamily(), GetType(), GetProtocol());
    if (INVALID_FD == fd)
    {
        PERROR("Failed to create socket");
        return INVALID_FD;
    }

    m_sockFd = fd;

    return m_sockFd;
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
VTcpSocket::VTcpSocket():
    VSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP),
    m_isConnected(false),
    m_isSrv(false),
    m_remoteAddr()
{
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
VTcpSocket::~VTcpSocket()
{
    m_isConnected = false;
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
void VTcpSocket::Close()
{
    m_isConnected = false;
    m_isSrv = false;
    
    return VSocket::Close();
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
int VTcpSocket::Send(const void* data, int size, const SocketAddress& to)
{
    (void)to;
    
    if ((NULL == data) || (size <= 0))
    {
        return -1;
    }

    if ((INVALID_FD == m_sockFd) || (!m_isConnected))
    {
        return -1;
    }
    
    int rc = 0;
    do
    {
        rc = ::send(m_sockFd, data, size, 0);
    }
    while ((rc < 0) && (EINTR == errno));

    if ((rc < 0) && (EAGAIN == errno))
    {
        return 0;
    }
       
    return rc;
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
int VTcpSocket::Recv(void* data, int size, SocketAddress& from)
{
    if ((NULL == data) || (size <= 0))
    {
        return -1;
    }

    if ((INVALID_FD == m_sockFd) || (!m_isConnected))
    {
        return -1;
    }

    from = m_remoteAddr;

    int rc = 0;
    do
    {
        rc = ::recv(m_sockFd, data, size, 0);
    }
    while ((rc < 0) && (EINTR == errno));

    // when link closed, return 0, not set errno
    // when no data and non-blocking, return -1 and set errno=EAGAIN
    // so we return -2 to show link is closed and return 0 to show no data.

    if (0 == rc)
    {
        return -2;
    }
    else if ((rc < 0) && (EAGAIN == errno))
    {
        return 0;
    }
    
    return rc;
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
int VTcpSocket::Send(const void* data, int size)
{
    SocketAddress to;
    return Send(data, size, to);
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
int VTcpSocket::Recv(void* data, int size)
{
    SocketAddress from;
    return Recv(data, size, from);
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
TcpClient::TcpClient():
    VTcpSocket()
{
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
TcpClient::TcpClient(int fd, SocketAddress& remoteAddress):
VTcpSocket()
{
    Set(fd, remoteAddress);
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
TcpClient::~TcpClient()
{
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
void TcpClient::Set(int fd, SocketAddress& remoteAddress)
{
    m_sockFd = fd;
    m_isConnected = true;
    m_remoteAddr = remoteAddress;
    m_remoteAddrString = m_remoteAddr.ToString();
    
    GetLocalAddress();
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
int TcpClient::Connect(const SocketAddress& remoteAddress)
{
    if (INVALID_FD == m_sockFd)
    {
        Create();
    }

    if (INVALID_FD == m_sockFd)
    {
        return -1;
    }

    m_isConnected = false;
    m_remoteAddr = remoteAddress;
    m_remoteAddrString = m_remoteAddr.ToString();
    
    int rc = 0;
    do
    {
        rc = ::connect(m_sockFd, m_remoteAddr, sizeof(struct sockaddr_in));
    }
    while ((rc < 0) && (EINTR == errno));

    if (0 == rc)
    {
        m_isConnected = true;

        int yes = 1;
        setsockopt(m_sockFd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes));

        GetLocalAddress();
    }

    return rc;
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
int TcpClient::Connect(const SocketAddress& remoteAddress, uint32_t waitMilliSeconds)
{
    if (INVALID_FD == m_sockFd)
    {
        Create();
    }

    if (INVALID_FD == m_sockFd)
    {
        return -1;
    }

    m_isConnected = false;
    m_remoteAddr = remoteAddress;
    m_remoteAddrString = m_remoteAddr.ToString();

    // set non-block mode
    int sockFlag = fcntl(m_sockFd, F_GETFL, 0);
    if (-1 == sockFlag)
    {
        return -1;
    }
    fcntl(m_sockFd, F_SETFL, sockFlag | O_NONBLOCK);

    do
    {
        // connect
        int rc = connect(m_sockFd, m_remoteAddr, sizeof(struct sockaddr_in));
        if (0 == rc)
        {
            m_isConnected = true;
            break;
        }
        else if (-1 == rc)
        {
            if (EINPROGRESS != errno)
            {
                break;
            }
        }

        bool waitIn = true;
        bool waitOut = true;
        rc = Wait(waitMilliSeconds, waitIn, waitOut);
        if (rc <= 0)
        {
            break;
        }
        
        int error = 0;
        socklen_t len = sizeof(int);
        if (0 == getsockopt(m_sockFd, SOL_SOCKET, SO_ERROR, &error, &len))
        {
            if (0 == error)
            {
                m_isConnected = true;
                break;
            }
        }
        
    }
    while(false);

    // reset socket flag
    fcntl(m_sockFd, F_SETFL, sockFlag);

    if (m_isConnected)
    {
        int yes = 1;
        setsockopt(m_sockFd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes));
        GetLocalAddress();
        
        return 0;
    }
    else
    {
        return -1;
    }
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
TcpServer::TcpServer():
    VTcpSocket()
{
    SetSrv(true);
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
TcpServer::~TcpServer()
{
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
int TcpServer::Listen(int waitNum)
{
    if (INVALID_FD == m_sockFd)
    {
        return -1;
    }

    int rc = ::listen(m_sockFd, waitNum);
    return rc;
}

//------------------------------------------------------------------------------
// This is a public API.
//------------------------------------------------------------------------------
int TcpServer::Accept(TcpClient& tcpClient)
{
    if (INVALID_FD == m_sockFd)
    {
        return -1;
    }

    socklen_t len = sizeof(struct sockaddr_in);
    SocketAddress from;

    int fd = 0;
    tcpClient.uAcceptTimes=0;
    do
    {
        fd = ::accept(m_sockFd, from, &len);
        ++tcpClient.uAcceptTimes;
        if(tcpClient.uAcceptTimes>5)
            break;
    }
    while ((fd < 0) && (EINTR == errno));

    if (fd < 0)
    {
        return -1;
    }
    
    tcpClient.Set(fd, from);
    tcpClient.SetSrv(true);
    
    m_isConnected = true;
    
    return 0;
}

int GetLocalIpAddrList(IfIpAddrList& list)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (fd < 0)
    {
        return -1;
    }
    
    char buf[1024] = "";
    struct ifreq* req = (struct ifreq*)buf;

    struct ifconf conf;
    conf.ifc_len = sizeof(buf);
    conf.ifc_buf = buf;

    ioctl(fd , SIOCGIFCONF, &conf);    //获取所有接口信息
     
    // 接下来一个一个的获取IP地址
    for (int i=(conf.ifc_len/sizeof(struct ifreq)); i>0; --i,++req)
    {
        if (AF_INET == req->ifr_flags)
        {
            IfIpAddr unit;
            unit.ifName = req->ifr_name;
            unit.ipAddr = inet_ntoa(((struct sockaddr_in*)&(req->ifr_addr))->sin_addr);

            list.push_back(unit);
        }
    }

    return 0;
}







