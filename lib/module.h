#pragma once
#ifndef MODULE_H
#define MODULE_H

#include <node.h>
#include <vector>
#include <unistd.h>
#include <cstdint>

namespace module {

    struct Module {
        uintptr_t start;
        uintptr_t end;
        char permissions[32];
        unsigned long long offset;
        unsigned dev_major;
        unsigned dev_minor;
        unsigned long long inode;
        char *pathname;

        ~Module() {
            delete pathname;
        }
    };

    Module findModule(const char* moduleName, pid_t processId, const char** errorMessage);
}

#endif
#pragma once
