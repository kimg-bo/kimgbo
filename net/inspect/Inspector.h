#ifndef INSPECT_INSPECTOR_H
#define INSPECT_INSPECTOR_H

#include <map>
#include <functional>
#include <memory>
#include <string.h>
#include "Mutex.h"
#include "HttpRequest.h"
#include "HttpServer.h"

namespace kimgbo
{
namespace net
{

class ProcessInspector;

// A internal inspector of the running process, usually a singleton.
class Inspector
{
 public:
  typedef std::vector<std::string> ArgList;
  typedef std::function<std::string (HttpRequest::Method, const ArgList& args)> Callback;
  
  Inspector(EventLoop* loop, const InetAddress& httpAddr, const string& name);
  void initialize();
  ~Inspector();

  void add(const std::string& module, const std::string& command, const Callback& cb, const std::string& help);

 private:
  typedef std::map<std::string, Callback> CommandList;
  typedef std::map<std::string, std::string> HelpList;

  void start();
  void onRequest(const HttpRequest& req, HttpResponse* resp);

  HttpServer m_server;
  std::unique_ptr<ProcessInspector> m_processInspector;
  MutexLock m_mutex;
  std::map<std::string, CommandList> m_commands;
  std::map<std::string, HelpList> m_helps;
};

}
}

#endif  // INSPECT_INSPECTOR_H
