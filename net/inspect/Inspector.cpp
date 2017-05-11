#include <string.h>
#include <functional>
#include <algorithm>
#include "Inspector.h"
#include "thread.h"
#include "EventLoop.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "ProcessInspector.h"

//#include <iostream>
//#include <iterator>
//#include <sstream>

using namespace kimgbo;
using namespace kimgbo::net;

namespace
{
Inspector* g_globalInspector = 0;

// Looks buggy
std::vector<std::string> split(const std::string& str)
{
  std::vector<std::string> result;
  size_t start = 0;
  size_t pos = str.find('/');
  while (pos != std::string::npos)
  {
    if (pos > start)
    {
      result.push_back(str.substr(start, pos-start));
    }
    start = pos+1;
    pos = str.find('/', start);
  }

  if (start < str.length())
  {
    result.push_back(str.substr(start));
  }

  return result;
}
}

Inspector::Inspector(EventLoop* loop, const InetAddress& httpAddr, const string& name)
    : m_server(loop, httpAddr, "Inspector:"+name),
      m_processInspector(new ProcessInspector)
{
  assert(CurrentThread::isMainThread());
  assert(g_globalInspector == 0);
  g_globalInspector = this;
  m_server.setHttpCallback(std::bind(&Inspector::onRequest, this, std::placeholders::_1, std::placeholders::_2));
  m_processInspector->registerCommands(this);
}

void Inspector::initialize()
{
	loop->runInLoop(std::bind(&Inspector::start, this));
}

Inspector::~Inspector()
{
  assert(CurrentThread::isMainThread());
  g_globalInspector = NULL;
}

void Inspector::add(const std::string& module, const std::string& command, const Callback& cb, const std::string& help)
{
  //MutexLockGuard lock(m_mutex); //此处做了一次优化
  m_commands[module][command] = cb;
  m_helps[module][command] = help;
}

void Inspector::start()
{
  m_server.start();
}

void Inspector::onRequest(const HttpRequest& req, HttpResponse* resp)
{
  if (req.path() == "/")
  {
    std::string result;
    MutexLockGuard lock(m_mutex);
    for (std::map<std::string, HelpList>::const_iterator helpListI = m_helps.begin();
         helpListI != m_helps.end();
         ++helpListI)
    {
      const HelpList& list = helpListI->second;
      for (HelpList::const_iterator it = list.begin();
           it != list.end();
           ++it)
      {
        result += "/";
        result += helpListI->first;
        result += "/";
        result += it->first;
        result += "\t";
        result += it->second;
        result += "\n";
      }
    }
    resp->setStatusCode(HttpResponse::k200Ok);
    resp->setStatusMessage("OK");
    resp->setContentType("text/plain");
    resp->setBody(result);
  }
  else
  {
    std::vector<std::string> result = split(req.path());
    //std::copy(result.begin(), result.end(), std::ostream_iterator<string>(std::cout, ", "));
    //std::cout << "\n";
    bool ok = false;
    if (result.size() == 0)
    {
    }
    else if (result.size() == 1)
    {
      std::string module = result[0];
    }
    else
    {
      std::string module = result[0];
      std::map<std::string, CommandList>::const_iterator commListI = m_commands.find(module);
      if (commListI != m_commands.end())
      {
        std::string command = result[1];
        const CommandList& commList = commListI->second;
        CommandList::const_iterator it = commList.find(command);
        if (it != commList.end())
        {
          ArgList args(result.begin()+2, result.end());
          if (it->second)
          {
            resp->setStatusCode(HttpResponse::k200Ok);
            resp->setStatusMessage("OK");
            resp->setContentType("text/plain");
            const Callback& cb = it->second;
            resp->setBody(cb(req.method(), args));
            ok = true;
          }
        }
      }
    }

    if (!ok)
    {
      resp->setStatusCode(HttpResponse::k404NotFound);
      resp->setStatusMessage("Not Found");
    }
    //resp->setCloseConnection(true);
  }
}

