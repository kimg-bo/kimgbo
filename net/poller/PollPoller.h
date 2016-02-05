#ifndef POLLPOLLER_H
#define POLLPOLLER_H

#include<map>
#include<vector>
#include"../Poller.h"

struct pollfd;

namespace kimgbo
{
	namespace net
	{
		///
		/// IO Multiplexing with poll(2).
		///
		class PollPoller:public Poller
		{
		public:
			PollPoller(EventLoop* loop);
			virtual ~PollPoller();
			
			virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels);
			virtual void updateChannel(Channel* channel);
			virtual void removeChannel(Channel* channel);
			
		private:
			void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
			typedef std::vector<struct pollfd> PollFdList;
			typedef std::map<int, Channel*> ChannelMap;
			PollFdList m_pollfds;
			ChannelMap m_channels;
		};		
	}
}

#endif