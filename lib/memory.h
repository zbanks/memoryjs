#pragma once
#ifndef MEMORY_H
#define MEMORY_H

#include <node.h>
#include <unistd.h>
#include <cstdint>
#include <cstring>

class memory {
public:
  memory();
  ~memory();

  template <class dataType>
  dataType readMemory(pid_t hProcess, uintptr_t address) {
    dataType cRead = {0};
    // XXX: process_vm_readv
    //ReadProcessMemory(hProcess, (LPVOID)address, &cRead, sizeof(dataType), NULL);
    return cRead;
  }

  char* readBuffer(pid_t hProcess, uintptr_t address, size_t size) {
    char* buffer = new char[size];
    memset(buffer, 0, size);
    // XXX: process_vm_readv
    //ReadProcessMemory(hProcess, (LPVOID)address, buffer, size, NULL);
    return buffer;
  }

  char readChar(pid_t hProcess, uintptr_t address) {
    char value = 0;
    // XXX: process_vm_readv
    //ReadProcessMemory(hProcess, (LPVOID)address, &value, sizeof(char), NULL);
    return value;
  }
};
#endif
