#include <node.h>
#include <vector>
#include "process.h"
#include "memoryjs.h"

process::process() {}
process::~process() {}

using v8::Exception;
using v8::Isolate;
using v8::String;

pid_t process::openProcess(const char* processName, const char** errorMessage){
  pid_t processPid = -1;

  /*
   * TODO: Find process!
  for (std::vector<PROCESSENTRY32>::size_type i = 0; i != processes.size(); i++) {
    // Check to see if this is the process we want.
    if (!strcmp(processes[i].szExeFile, processName)) {
      handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processes[i].th32ProcessID);
      process = processes[i];
      break;
    }
  }
  */

  if (processPid < 0) {
    *errorMessage = "unable to find process";
  }

  return processPid;
}
