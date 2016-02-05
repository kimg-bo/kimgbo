#include "PollPoller.h"
#include "Logging.h"
//#include <muduo/base/Types.h>
#include "Channel.h"
#include <assert.h>
#include <poll.h>

using namespace kimgbo;
using namespace kimgbo::net;

PollPoller::PollPoller(EventLoop* loop)
  : Poller(loop)
{
}

PollPoller::~PollPoller()
{
}

Timestamp PollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
  // XXX pollfds_ shouldn't change
  int numEvents = ::poll(&*m_pollfds.begin(), m_pollfds.size(), timeoutMs);
  Timestamp now(Timestamp::now());
  if (numEvents > 0)
  {
    LOG_TRACE << numEvents << " events happended";
    //std::cout << numEvents << " events happended";
    fillActiveChannels(numEvents, activeChannels);
  }
  else if (numEvents == 0)
  {
    LOG_TRACE << " nothing happended";
    //std::cout << " nothing happended";
  }
  else
  {
    LOG_SYSERR << "PollPoller::poll()";
    //std::cout << "PollPoller::poll()";
  }
  return now;
}

void PollPoller::fillActiveChannels(int numEvents,
                                    ChannelList* activeChannels) const
{
  for (PollFdList::const_iterator pfd = m_pollfds.begin();
      pfd != m_pollfds.end() && numEvents > 0; ++pfd)
  {
    if (pfd->revents > 0)
    {
      --numEvents;
      ChannelMap::const_iterator ch = m_channels.find(pfd->fd);
      assert(ch != m_channels.end());
      Channel* channel = ch->second;
      assert(channel->fd() == pfd->fd);
      channel->set_revents(pfd->revents);
      // pfd->revents = 0;
      activeChannels->push_back(channel);
    }
  }
}

void PollPoller::updateChannel(Channel* channel)
{
  Poller::assertInLoopThread();
  LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();
  //std::cout << "fd = " << channel->fd() << " events = " << channel->events();
  if (channel->index() < 0)
  {
    // a new one, add to pollfds_
    assert(m_channels.find(channel->fd()) == m_channels.end());
    struct pollfd pfd;
    pfd.fd = channel->fd();
    pfd.events = static_cast<short>(channel->events());
    pfd.revents = 0;
    m_pollfds.push_back(pfd);
    int idx = static_cast<int>(m_pollfds.size())-1;
    channel->set_index(idx);
    m_channels[pfd.fd] = channel;
  }
  else
  {
    // update existing one
    assert(m_channels.find(channel->fd()) != m_channels.end());
    assert(m_channels[channel->fd()] == channel);
    int idx = channel->index();
    assert(0 <= idx && idx < static_cast<int>(m_pollfds.size()));
    struct pollfd& pfd = m_pollfds[idx];
    assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd()-1);
    pfd.events = static_cast<short>(channel->events());
    pfd.revents = 0;
    if (channel->isNoneEvent())
    {
      // ignore this pollfd
      pfd.fd = -channel->fd()-1;
    }
  }
}

void PollPoller::removeChannel(Channel* channel)
{
  Poller::assertInLoopThread();
  LOG_TRACE << "fd = " << channel->fd();
  //std::cout << "fd = " << channel->fd();
  assert(m_channels.find(channel->fd()) != m_channels.end());
  assert(m_channels[channel->fd()] == channel);
  assert(channel->isNoneEvent());
  int idx = channel->index();
  assert(0 <= idx && idx < static_cast<int>(m_pollfds.size()));
  const struct pollfd& pfd = m_pollfds[idx]; (void)pfd;
  assert(pfd.fd == -channel->fd()-1 && pfd.events == channel->events());
  size_t n = m_channels.erase(channel->fd());
  assert(n == 1); (void)n;
  if (implicit_cast<size_t>(idx) == m_pollfds.size()-1)
  {
    m_pollfds.pop_back();
  }
  else
  {
    int channelAtEnd = m_pollfds.back().fd;
    iter_swap(m_pollfds.begin()+idx, m_pollfds.end()-1);
    if (channelAtEnd < 0)
    {
      channelAtEnd = -channelAtEnd-1;   
    }
    m_channels[channelAtEnd]->set_index(idx);
    m_pollfds.pop_back();
  }
}


