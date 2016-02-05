#ifndef INSPECT_PROCESSINSPECTOR_H
#define INSPECT_PROCESSINSPECTOR_H

#include <string>
#include "Inspector.h"
//#include "Types.h"

namespace kimgbo
{
namespace net
{

class ProcessInspector
{
 public: 	
  void registerCommands(Inspector* ins);

 private:
  static std::string pid(HttpRequest::Method, const Inspector::ArgList&);
  static std::string procStatus(HttpRequest::Method, const Inspector::ArgList&);
  static std::string openedFiles(HttpRequest::Method, const Inspector::ArgList&);
  static std::string threads(HttpRequest::Method, const Inspector::ArgList&);
};

}
}

#endif  // INSPECT_PROCESSINSPECTOR_H
