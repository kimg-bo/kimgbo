#ifndef EPOLLPOLLER_H
#define EPOLLPOLLER_H

#include<map>
#include<vector>
#include"../Poller.h"

struct epoll_event;

namespace kimgbo
{
	namespace net
	{
		class EPollPoller:public Poller
		{
		public:
			EPollPoller(EventLoop* loop);
  		virtual ~EPollPoller();
			
			virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels);
  		virtual void updateChannel(Channel* channel);
  		virtual void removeChannel(Channel* channel);
  		
  	private:
  		static const int kInitEventListSize = 16;
  		void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
  		void update(int operation, Channel* channel);
  		typedef std::vector<struct epoll_event> EventList;
  		typedef std::map<int, Channel*> ChannelMap;
			
			int m_epollfd;
			EventList m_events;
			ChannelMap m_channels;
		};
	}	
}

#endif