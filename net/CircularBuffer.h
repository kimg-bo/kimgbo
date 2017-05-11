#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <assert.h>
#include <string.h>
#include <algorithm>
#include <vector>

#include "StringPiece.h"
#include "Types.h"
#include "Endian.h"

namespace kimgbo
{
	namespace net
	{
		class CircularBuffer
		{
		public:
			static const size_t kCheapPrepend = 8;
			static const size_t kInitialSize = 1024;
			
			CircularBuffer():m_buffer(kCheapPrepend+kInitialSize), m_readerIndex(kCheapPrepend), m_writerIndex(kCheapPrepend),
				 m_bufferActuCpct(kInitialSize), m_hasData(false)
			{
				assert(readableBytes() == 0);
				assert(writableBytes() == kInitialSize);
				assert(prependableBytes() == kCheapPrepend);
				assert(m_hasData == false);
			}
			
			void swap(CircularBuffer& rhs)
			{
				m_buffer.swap(rhs.m_buffer);
				std::swap(m_readerIndex, rhs.m_readerIndex);
    		std::swap(m_writerIndex, rhs.m_writerIndex);
    		std::swap(m_bufferActuCpct, rhs.m_bufferActuCpct);
    		std::swap(m_hasData, rhs.m_hasData);
			}
			
			size_t readableBytes() const
  		{
  			if(!m_hasData)
  				return 0;
  			
  			if(m_writerIndex > m_readerIndex)
  			{
  				return m_writerIndex - m_readerIndex;
  			}
  			else if(m_writerIndex == m_readerIndex)
  			{
  				return m_bufferActuCpct; 
  			}
  			else
  			{
  				return m_bufferActuCpct - (m_readerIndex - m_writerIndex);
  			}
  		}

  		size_t writableBytes() const
  		{
  			if(!m_hasData)
  				return m_bufferActuCpct;
  			
  			if(m_writerIndex > m_readerIndex)
  			{
  				return m_bufferActuCpct - (m_writerIndex - m_readerIndex);
  			}
  			else if(m_writerIndex == m_readerIndex)
  			{
  				return 0;
  			}
  			else
  			{
  				return m_readerIndex - m_writerIndex;
  			}
  		}

  		size_t prependableBytes() const
  		{
  			if(!m_hasData)
  				kCheapPrepend;
  			
  			if(m_writerIndex > m_readerIndex)
  			{
  				return m_readerIndex;
  			}
  			else 
  			{
  				return kCheapPrepend;
  			}
  		}

  		const char* peek() const
  		{
  			return begin() + m_readerIndex; 
  		}

  		const char* findCRLF() const
  		{
  			if(!m_hasData)
  				return NULL;
  			
  			const char* crlf = NULL;
  				
  			if(m_writerIndex > m_readerIndex)
  			{
  				crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF+2);
  			}
    		else
    		{
    			crlf = std::search(peek(), end(), kCRLF, kCRLF+2);
    			if(crlf == NULL)
    			{
    				crlf = std::search(continueRead(), beginWrite(), kCRLF, kCRLF+2);
    			}
    		}
    		return crlf == beginWrite() ? NULL : crlf;
  		}
  		
  		const char* findCRLF(const char* start) const
  		{
    		if(!m_hasData)
  				return NULL;
  			
  			const char* crlf = NULL;
    		if(beginWrite() >= start)
    		{
    			crlf = std::search(start, beginWrite(), kCRLF, kCRLF+2);
    		}
    		else
    		{
    			crlf = std::search(start, end(), kCRLF, kCRLF+2);
    			if(crlf == end())
    			{
    				crlf = std::search(continueRead(), beginWrite(), kCRLF, kCRLF+2);
    			}
    		}
    		return crlf == beginWrite() ? NULL : crlf;
  		}
  		
  		void retrieve(size_t len)
  		{
  			assert(len <= readableBytes());
  			if(len < readableBytes())
  			{
  				m_readerIndex += len;
  				if(m_readerIndex >= (m_bufferActuCpct + kCheapPrepend))
  				{
  					m_readerIndex = m_readerIndex % m_bufferActuCpct;
  				}
  			}
  			else
  			{
  				retrieveAll();
				}
  		}
  		
  		void retrieveUntil(const char* end)
  		{
  			if(!m_hasData)
  				return;
  			
  			size_t len;
  			if(m_writerIndex <= m_readerIndex)
  			{
  				if(end <= beginWrite())
  				{
  					assert(end >= continueRead());
  					len = m_bufferActuCpct + kCheapPrepend - m_readerIndex;
  					len = len + (end - continueRead());
  				}
  				else
  				{
  					assert(end >= peek());
  					len = end - peek();
  				}
  				retrieve(len);
  			}
  			else if(m_writerIndex > m_readerIndex)
  			{
  				assert(peek() <= end);
  				assert(end <= beginWrite());
  				retrieve(end - peek());
  			}
  		}
  		
  		void retrieveInt32()
  		{
  			retrieve(sizeof(int32_t));
  		}
  		
  		void retrieveInt16()
  		{
  			retrieve(sizeof(int16_t));
  		}
  		
  		void retrieveInt8()
  		{
  			retrieve(sizeof(int8_t));
  		}
  		
  		void retrieveAll()
  		{
  			m_readerIndex = kCheapPrepend;
    		m_writerIndex = kCheapPrepend;
    		m_hasData = false;
  		}
  		
  		string retrieveAllAsString()
  		{
    		return retrieveAsString(readableBytes());
    		//LOG_INFO << "retrieveAllAsString--- " << "m_readerIndex: " << m_readerIndex << ", m_writerIndex: " << m_writerIndex;
  		}

  		string retrieveAsString(size_t len)
  		{
    		assert(len <= readableBytes());
    		
    		string result;
    		if((m_readerIndex < m_writerIndex) || (m_readerIndex + len) <= (m_bufferActuCpct + kCheapPrepend))
    		{
    			result.append(peek(), len);
    			retrieve(len);
    		}
    		else
    		{
    			size_t toWriteLen = m_bufferActuCpct + kCheapPrepend - m_readerIndex;
    			result.append(peek(), toWriteLen);
    			result.append(continueRead(), len - toWriteLen);
    			retrieve(len);
    		}
    		if(readableBytes() == 0)
    			m_hasData = false;
    			
    		return result;
  		}
  		
  		StringPiece toStringPiece() const
  		{
  			if(m_readerIndex < m_writerIndex)
  			{
    			return StringPiece(peek(), static_cast<int>(readableBytes()));
    		}
    		else
    		{
    			if(m_hasData)
    			{
    				string result(peek(), (m_bufferActuCpct + kCheapPrepend - m_readerIndex));
    				result.append(continueRead(), (m_writerIndex - kCheapPrepend + 1));
    				return StringPiece(&*result.begin(), static_cast<int>(readableBytes()));
    			}
    			return StringPiece(peek(), static_cast<int>(readableBytes()));
    		}
  		}

  		void append(const StringPiece& str)
  		{
    		append(str.data(), str.size());
  		}
  		
  		void append(const char* /*restrict*/ data, size_t len)
  		{
  			ensureWritableBytes(len);
  			
  			if(m_writerIndex > m_readerIndex)
  			{
  				if((m_writerIndex + len) <= (m_bufferActuCpct + kCheapPrepend))
  				{
  					std::copy(data, data+len, beginWrite());
  				}
  				else
  				{
  					size_t toWriteLen = m_bufferActuCpct + kCheapPrepend - m_writerIndex;
    				std::copy(data, data+toWriteLen, beginWrite());
    				std::copy(data+toWriteLen, data+len, continueRead());
  				}
  			}
    		else
    		{
    			std::copy(data, data+len, beginWrite());
    		}
  			hasWritten(len);
  			
  			//LOG_INFO << "append--- " << "m_readerIndex: " << m_readerIndex << ", m_writerIndex: " << m_writerIndex;
  		}
  		
  		void append(const void* /*restrict*/ data, size_t len)
  		{ 
    		append(static_cast<const char*>(data), len);
  		}
  		
  		void ensureWritableBytes(size_t len)
  		{
  			if(writableBytes() < len)
  			{
  				makeSpace(len);
  			}
  			
  			assert(writableBytes() >= len);
  		}
  		
  		char* beginWrite()
  		{
  			return begin()+m_writerIndex;
  		}
  		
  		const char* beginWrite() const
  		{
  			return begin()+m_writerIndex;
  		}
  		
  		void hasWritten(size_t len)
  		{
  			m_writerIndex += len;
  			if(m_writerIndex >= (m_bufferActuCpct + kCheapPrepend))
  			{
  				m_writerIndex = m_writerIndex % m_bufferActuCpct;
  			}
  			if(len > 0)
  			{
  				m_hasData = true;
  			}
  		}
  		
  		///
  		/// Append int32_t using network endian
  		///
  		
  		void appendInt32(int32_t x)
  		{
    		int32_t be32 = sockets::hostToNetwork32(x);
    		append(&be32, sizeof(be32));
  		}

  		void appendInt16(int16_t x)
  		{
    		int16_t be16 = sockets::hostToNetwork16(x);
    		append(&be16, sizeof(be16));
  		}

  		void appendInt8(int8_t x)
  		{
    		append(&x, sizeof(x));
  		}
  		
  		///
  		/// Read int32_t from network endian
  		///
  		/// Require: buf->readableBytes() >= sizeof(int32_t)
  		int32_t readInt32()
  		{
    		int32_t result = peekInt32();
    		retrieveInt32();
    		return result;
  		}

  		int16_t readInt16()
  		{
    		int16_t result = peekInt16();
    		retrieveInt16();
    		return result;
  		}

  		int8_t readInt8()
  		{
    		int8_t result = peekInt8();
    		retrieveInt8();
    		return result;
  		}
  		
  		///
  		/// Peek int32_t from network endian
  		///
  		/// Require: buf->readableBytes() >= sizeof(int32_t)
			int32_t peekInt32() const
			{
				assert(readableBytes() >= sizeof(int32_t));
				int32_t be32 = 0;
				::memcpy(&be32, peek(), sizeof(be32));
				return sockets::networkToHost32(be32);
			}
			
			int16_t peekInt16() const
  		{
    		assert(readableBytes() >= sizeof(int16_t));
    		int16_t be16 = 0;
    		::memcpy(&be16, peek(), sizeof be16);
    		return sockets::networkToHost16(be16);
  		}

  		int8_t peekInt8() const
  		{
    		assert(readableBytes() >= sizeof(int8_t));
    		int8_t x = *peek();
    		return x;
  		}
			
			///
  		/// Prepend int32_t using network endian
  		///
  		void prependInt32(int32_t x)
  		{
    		int32_t be32 = sockets::hostToNetwork32(x);
    		prepend(&be32, sizeof be32);
  		}

  		void prependInt16(int16_t x)
  		{
    		int16_t be16 = sockets::hostToNetwork16(x);
    		prepend(&be16, sizeof be16);
  		}

  		void prependInt8(int8_t x)
  		{
    		prepend(&x, sizeof x);
  		}
  		
  		void prepend(const void* /*restrict*/ data, size_t len)
  		{
  			assert(len <= prependableBytes());
  			m_readerIndex -= len;
  			const char* d = static_cast<const char*>(data);
  			std::copy(d, d+len, begin()+m_readerIndex);
  		}
  		
  		void shrink(size_t reserve)
  		{
  			CircularBuffer other;
  			other.ensureWritableBytes(readableBytes() + reserve);
  			other.append(toStringPiece());
  			swap(other);
  		}
  		
  		ssize_t readFd(int fd, int* savedErrno);
			
		private:
			char* begin()
			{
				return &*m_buffer.begin();
			}
			
			const char* begin() const
			{
				return &*m_buffer.begin();
			}
			
			char* end()
			{
				return &*m_buffer.end();
			}
			
			const char* end() const
			{
				return &*m_buffer.end();
			}
			
			char* continueRead()
			{
				return begin()+kCheapPrepend;
			}
			
			const char* continueRead() const
			{
				return begin()+kCheapPrepend;
			}
			
			void makeSpace(size_t len)
			{
				if(m_writerIndex > m_readerIndex)
				{
					m_buffer.resize(m_writerIndex+len);
      		m_bufferActuCpct = m_buffer.size() - kCheapPrepend;
      		
      		//LOG_INFO << "makeSpace--- " << "m_writerIndex+len: " << m_writerIndex+len;
    			//LOG_INFO << "makeSpace--- " << "m_buffer.size: " << m_buffer.size();
				}
      	else if(!m_hasData)
      	{
      		m_buffer.resize(len + kCheapPrepend);
      		m_bufferActuCpct = m_buffer.size() - kCheapPrepend;
      		m_readerIndex = kCheapPrepend;
      		m_writerIndex = kCheapPrepend;
      		
      		//LOG_INFO << "makeSpace--- " << "m_buffer.size: " << m_buffer.size();
      	}
      	else
      	{
      		CircularBuffer other;
  				other.ensureWritableBytes(readableBytes() + len);
  				
  				//LOG_INFO << "makeSpace--- " << "toStringPiece: " << toStringPiece();
  				
  				other.append(toStringPiece());
  				swap(other);
  				
  				//LOG_INFO << "makeSpace--- " << "m_buffer.size: " << m_buffer.size();
      	}
      	
    		//LOG_INFO << "makeSpace--- " << "m_bufferActuCpct: " << m_bufferActuCpct;
			}
			
		private:
			std::vector<char> m_buffer;
			size_t m_readerIndex;
			size_t m_writerIndex;
			size_t m_bufferActuCpct;//除去预留的空间外，实际可用的空间
			bool m_hasData;
			
			static const char kCRLF[];
		};
	}
}


#endif