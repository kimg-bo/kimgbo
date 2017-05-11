#ifndef BUFFER_H
#define BUFFER_H

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
		class Buffer
		{
		public:
			static const size_t kCheapPrepend = 8;
			static const size_t kInitialSize = 1024;
			
			Buffer():m_buffer(kCheapPrepend+kInitialSize), m_readerIndex(kCheapPrepend), m_writerIndex(kCheapPrepend)
			{
				assert(readableBytes() == 0);
				assert(writableBytes() == kInitialSize);
				assert(prependableBytes() == kCheapPrepend);
			}
			
			void swap(Buffer& rhs)
			{
				m_buffer.swap(rhs.m_buffer);
				std::swap(m_readerIndex, rhs.m_readerIndex);
    		std::swap(m_writerIndex, rhs.m_writerIndex);
			}
			
			size_t readableBytes() const
  		{
  			return m_writerIndex - m_readerIndex;
  		}

  		size_t writableBytes() const
  		{
  			return m_buffer.size() - m_writerIndex;
  		}

  		size_t prependableBytes() const
  		{
  			return m_readerIndex; 
  		}

  		const char* peek() const
  		{
  			return begin() + m_readerIndex; 
  		}

  		const char* findCRLF() const
  		{
    		const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF+2);
    		return crlf == beginWrite() ? NULL : crlf;
  		}
  		
  		const char* findCRLF(const char* start) const
  		{
    		assert(peek() <= start);
    		assert(start <= beginWrite());
    		const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF+2);
    		return crlf == beginWrite() ? NULL : crlf;
  		}
  		
  		void retrieve(size_t len)
  		{
  			assert(len <= readableBytes());
  			if(len < readableBytes())
  			{
  				m_readerIndex += len;
  			}
  			else
  			{
  				retrieveAll();
  			}
  		}
  		
  		void retrieveUntil(const char* end)
  		{
  			assert(peek() <= end);
  			assert(end <= beginWrite());
  			retrieve(end - peek());
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
  		}
  		
  		string retrieveAllAsString()
  		{
    		return retrieveAsString(readableBytes());
  		}

  		string retrieveAsString(size_t len)
  		{
    		assert(len <= readableBytes());
    		string result(peek(), len);
    		retrieve(len);
    		return result;
  		}
  		
  		StringPiece toStringPiece() const
  		{
    		return StringPiece(peek(), static_cast<int>(readableBytes()));
  		}

  		void append(const StringPiece& str)
  		{
    		append(str.data(), str.size());
  		}
  		
  		void append(const char* /*restrict*/ data, size_t len)
  		{
  			ensureWritableBytes(len);
  			std::copy(data, data+len, beginWrite());
  			hasWritten(len);
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
  			Buffer other;
  			other.ensureWritableBytes(readableBytes()+reserve);
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
			
			void makeSpace(size_t len)
			{
				if (writableBytes() + prependableBytes() < len + kCheapPrepend)
    		{
      		// FIXME: move readable data
      		m_buffer.resize(m_writerIndex+len);
    		}
    		else
    		{
      		// move readable data to the front, make space inside buffer
      		assert(kCheapPrepend < m_readerIndex);
      		size_t readable = readableBytes();
      		std::copy(begin()+m_readerIndex, begin()+m_writerIndex, begin()+kCheapPrepend);
      		m_readerIndex = kCheapPrepend;
      		m_writerIndex = m_readerIndex + readable;
      		assert(readable == readableBytes());
    		}
			}
			
		private:
			std::vector<char> m_buffer;
			size_t m_readerIndex;
			size_t m_writerIndex;
			
			static const char kCRLF[];
		};
	}
}


#endif