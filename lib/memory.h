#pragma once
#ifndef MEMORY_H
#define MEMORY_H

#include <node.h>

class memory {
public:
  memory();
  ~memory();

  template <class dataType>
  dataType readMemory(HANDLE hProcess, DWORD64 address) {
    dataType cRead;
    ReadProcessMemory(hProcess, (LPVOID)address, &cRead, sizeof(dataType), NULL);
    return cRead;
  }

  char* readBuffer(HANDLE hProcess, DWORD64 address, SIZE_T size) {
    char* buffer = new char[size];
    ReadProcessMemory(hProcess, (LPVOID)address, buffer, size, NULL);
    return buffer;
  }

  char readChar(HANDLE hProcess, DWORD64 address) {
    char value;
    ReadProcessMemory(hProcess, (LPVOID)address, &value, sizeof(char), NULL);
    return value;
  }
};
#endif
