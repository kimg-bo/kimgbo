#ifndef INETADDRESS_H
#define INETADDRESS_H

#include<stdint.h>
#include<netinet/in.h>
#include"StringPiece.h"

namespace kimgbo
{
	namespace net
	{
		class InetAddress
		{
		public:
			explicit InetAddress(uint16_t port);
			
			InetAddress(const StringPiece& ip, uint16_t port);
			
			InetAddress(const struct sockaddr_in& addr):m_addr(addr)
			{
			}
			
			string toIp() const;
			string toIpPort() const;
			string toHostPort() const __attribute__ ((deprecated))
			{
				return toIpPort();
			}
			
			const struct sockaddr_in& getSockAddrInet() const { return m_addr; }
			void setSockAddrInet(const struct sockaddr_in& addr) { m_addr = addr; }
			
			uint32_t ipNetEndian() const { return m_addr.sin_addr.s_addr; }
			uint16_t portNetEndian() const { return m_addr.sin_port; }
		
		private:
			struct sockaddr_in m_addr;
		};
	}
}

#endif