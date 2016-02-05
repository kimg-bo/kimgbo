#include<string.h>
#include<netinet/in.h>
#include"InetAddress.h"
#include"Endian.h"
#include"SocketOps.h"

// INADDR_ANY use (type)value casting.
#pragma GCC diagnostic ignored "-Wold-style-cast"
static const in_addr_t kInaddrAny = INADDR_ANY;
#pragma GCC diagnostic error "-Wold-style-cast"

using namespace kimgbo;
using namespace kimgbo::net;
	
InetAddress::InetAddress(uint16_t port)
{
	bzero(&m_addr, sizeof(m_addr));
	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.s_addr = sockets::hostToNetwork32(kInaddrAny);
	m_addr.sin_port = sockets::hostToNetwork16(port);
}
			
InetAddress::InetAddress(const StringPiece& ip, uint16_t port)
{
	bzero(&m_addr, sizeof(m_addr));
	sockets::fromIpPort(ip.data(), port, &m_addr);
}

string InetAddress::toIp() const
{
	char buf[32];
  sockets::toIpPort(buf, sizeof(buf), m_addr);
  return buf;
}

string InetAddress::toIpPort() const
{
	char buf[32];
  sockets::toIp(buf, sizeof(buf), m_addr);
  return buf;
}