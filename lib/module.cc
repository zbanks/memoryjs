#include <node.h>
#include <vector>
#include "module.h"
#include "process.h"
#include "memoryjs.h"

uintptr_t module::findModule(const char* moduleName, pid_t processId, const char** errorMessage) {
  uintptr_t module = 0;
  bool found = false;

  /*
   * TODO: Find DLL!
  std::vector<MODULEENTRY32> moduleEntries = getModules(processId, errorMessage);

  // Loop over every module
  for (std::vector<MODULEENTRY32>::size_type i = 0; i != moduleEntries.size(); i++) {
    // Check to see if this is the module we want.
    if (!strcmp(moduleEntries[i].szModule, moduleName)) {
      // module is returned and moduleEntry is used internally for reading/writing to memory
      module = moduleEntries[i];
      found = true;
      break;
    }
  }
  */

  if (!found) {
    *errorMessage = "unable to find module";
  }

  return module;
} 
