#pragma once
#ifndef MODULE_H
#define MODULE_H

#include <node.h>
#include <vector>

namespace module {
  DWORD64 getBaseAddress(const char* processName, DWORD processId);
  MODULEENTRY32 findModule(const char* moduleName, DWORD processId, char** errorMessage);
  std::vector<MODULEENTRY32> getModules(DWORD processId, char** errorMessage);
  std::vector<THREADENTRY32> getThreads(DWORD processId, char** errorMessage);

};
#endif
#pragma once
