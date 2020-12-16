#pragma once
#ifndef MODULE_H
#define MODULE_H

#include <node.h>
#include <vector>
#include <unistd.h>
#include <cstdint>

namespace module {
  uintptr_t findModule(const char* moduleName, pid_t processId, const char** errorMessage);
};
#endif
#pragma once
