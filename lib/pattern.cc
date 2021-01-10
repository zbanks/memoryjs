#include <node.h>
#include <vector>
#include <cerrno>
#include <cstring>
#include <sys/uio.h>
#include "module.h"
#include "pattern.h"
#include "memoryjs.h"
#include "process.h"
#include "memory.h"

#define INRANGE(x,a,b) (x >= a && x <= b) 
#define getBits( x ) (INRANGE(x,'0','9') ? (x - '0') : ((x&(~0x20)) - 'A' + 0xa))
#define getByte( x ) (getBits(x[0]) << 4 | getBits(x[1]))

pattern::pattern() {}
pattern::~pattern() {}

/* based off Y3t1y3t's implementation */
uintptr_t pattern::findPattern(pid_t hProcess, module::Module module, const char* pattern, short sigType, uintptr_t patternOffset, uintptr_t addressOffset) { 
  auto moduleSize = uintptr_t(module.end - module.start);
  auto moduleBase = uintptr_t(module.start);

  auto moduleBytes = std::vector<unsigned char>(moduleSize);

  struct iovec remote_iov = {.iov_base = (void *)moduleBase, .iov_len = moduleSize};
  struct iovec local_iov = {.iov_base = &moduleBytes[0], .iov_len = moduleSize};

  ssize_t rc = process_vm_readv(hProcess, &local_iov, 1, &remote_iov, 1, 0);

  if (rc < 0 || (size_t) rc != moduleSize) {
    printf("error: process_vm_readv returned %zd instead of %zu (%s) address=%#lx\n", rc, moduleSize, strerror(errno), moduleBase);
  }

  auto byteBase = const_cast<unsigned char*>(&moduleBytes.at(0));
  auto maxOffset = moduleSize - 0x1000;

  for (auto offset = 0UL; offset < maxOffset; ++offset) {
    if (compareBytes(byteBase + offset, pattern)) {
      auto address = moduleBase + offset + patternOffset;
      struct iovec remote_iov = {.iov_base = (void *)address, .iov_len = sizeof(uintptr_t)};
      struct iovec local_iov = {.iov_base = &address, .iov_len = sizeof(uintptr_t)};

      /* read memory at pattern if flag is raised*/
      if (sigType & ST_READ) rc = process_vm_readv(hProcess, &local_iov, 1, &remote_iov, 1, 0);

      if (rc < 0 || (size_t) rc != moduleSize) {
        printf("error: process_vm_readv returned %zd instead of %zu (%s) address=%#lx\n", rc, moduleSize, strerror(errno), moduleBase);
      }

      /* subtract image base if flag is raised */
      if (sigType & ST_SUBTRACT) address -= moduleBase;

      return address + addressOffset;
    }
  }

  // the method that calls this will check to see if the value is -2
	// and throw a 'no match' error
  return -2;
};

bool pattern::compareBytes(const unsigned char* bytes, const char* pattern) {
  for (; *pattern; *pattern != ' ' ? ++bytes : bytes, ++pattern) {
    if (*pattern == ' ' || *pattern == '?')
      continue;
		
    if (*bytes != getByte(pattern))
      return false;
    
    ++pattern;
  }
  
  return true;
}
