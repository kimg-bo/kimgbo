#ifndef POLLER_H
#define POLLER_H

#include <vector>
#include "../base/Timestamp.h"
#include "EventLoop.h"
#include "Channel.h"

namespace kimgbo
{
	namespace net
	{
		//class Channel;
		
		class Poller
		{
		public:
			typedef std::vector<Channel*> ChannelList;
			
			Poller(EventLoop* loop);
			virtual ~Poller();
			
			/// Polls the I/O events.
  		/// Must be called in the loop thread.
			virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;
			/// Changes the interested I/O events.
  		/// Must be called in the loop thread.
			virtual void updateChannel(Channel* channel) = 0;
			/// Remove the channel, when it destructs.
  		/// Must be called in the loop thread.
			virtual void removeChannel(Channel* channel) = 0;
			
			static Poller* newDefaultPoller(EventLoop* loop);
			
			void assertInLoopThread()
			{
				m_ownerLoop->assertInLoopThread();
			}
			
		private:
			EventLoop* m_ownerLoop; //À˘ ÙEventLoop
		};
	}
}

#endif