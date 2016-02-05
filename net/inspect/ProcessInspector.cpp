#include <stdio.h>
#include "ProcessInspector.h"
#include "ProcessInfo.h"

using namespace kimgbo;
using namespace kimgbo::net;

void ProcessInspector::registerCommands(Inspector* ins)
{
  ins->add("proc", "pid", ProcessInspector::pid, "print pid");
  ins->add("proc", "status", ProcessInspector::procStatus, "print /proc/self/status");
  ins->add("proc", "opened_files", ProcessInspector::openedFiles, "count /proc/self/fd");
  ins->add("proc", "threads", ProcessInspector::threads, "list /proc/self/task");
}

std::string ProcessInspector::pid(HttpRequest::Method, const Inspector::ArgList&)
{
  char buf[32];
  snprintf(buf, sizeof(buf), "%d", ProcessInfo::pid());
  return buf;
}

std::string ProcessInspector::procStatus(HttpRequest::Method, const Inspector::ArgList&)
{
  return ProcessInfo::procStatus();
}

std::string ProcessInspector::openedFiles(HttpRequest::Method, const Inspector::ArgList&)
{
  char buf[32];
  snprintf(buf, sizeof buf, "%d", ProcessInfo::openedFiles());
  return buf;
}

std::string ProcessInspector::threads(HttpRequest::Method, const Inspector::ArgList&)
{
  std::vector<pid_t> threads = ProcessInfo::threads();
  std::string result;
  for (size_t i = 0; i < threads.size(); ++i)
  {
    char buf[32];
    snprintf(buf, sizeof buf, "%d\n", threads[i]);
    result += buf;
  }
  return result;
}

