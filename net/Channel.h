#ifndef CHANNEL_H
#define CHANNEL_H

#include <iostream>
#include <functional>
#include <tr1/memory>
#include "Timestamp.h"
//#include "Types.h"

namespace kimgbo
{
	namespace net
	{
		class EventLoop;
		
		class Channel
		{
		public:
			typedef std::function<void()> EventCallback;
			typedef std::function<void(Timestamp)> ReadEventCallback;
				
			Channel(EventLoop* loop, int fd);
			~Channel();
			
			void handleEvent(Timestamp recieveTime);
			
			void setReadCallback(const ReadEventCallback& cb)
			{ m_readCallback = cb; }
			void setWriteCallback(const EventCallback& cb)
			{ m_writeCallback = cb; }
			void setCloseCallback(const EventCallback& cb)
			{ m_closeCallback = cb; }
			void setErrorCallback(const EventCallback& cb)
			{ m_errorCallback = cb; }
			
			void tie(const std::tr1::shared_ptr<void>& obj);
				
			int fd() const { return m_fd; }
			int events() const { return m_events; }
			void set_revents(int revt) { m_revents = revt; }
			bool isNoneEvent() const { return m_events == kNoneEvent; }
			void enableReading() { m_events |= kReadEvent; update(); }
			void enableWriting() { m_events |= kWriteEvent; update(); }
  		void disableWriting() { m_events &= ~kWriteEvent; update(); }
  		void disableAll() { m_events = kNoneEvent; update(); }
  		bool isWriting() const { return m_events & kWriteEvent; }
  		
  		int index() { return m_index; }
  		void set_index(int idx) { m_index = idx; }
  		std::string reventsToString() const;
  		void doNotLogHup() { m_logHup = false; }
  		EventLoop* ownerLoop() { return m_loop; }
  		void remove();
			
		private:
			void update();
			void handleEventWithGuard(Timestamp receiveTime);
			
			static const int kNoneEvent;
			static const int kReadEvent;
			static const int kWriteEvent;
			
			EventLoop* m_loop;
			const int m_fd;
			int m_events;
			int m_revents;
			int m_index;
			int m_logHup;
			std::tr1::weak_ptr<void> m_tie;
			bool m_tied;
			bool m_eventHandling;
	  	
	  	ReadEventCallback m_readCallback;
  		EventCallback m_writeCallback;
  		EventCallback m_closeCallback;
  		EventCallback m_errorCallback;		
		};
	}
}


#endif