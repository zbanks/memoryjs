#include <node.h>
#include <vector>
#include <dirent.h>
#include <cstring>
#include "process.h"
#include "memoryjs.h"
#include <sys/stat.h>

process::process() {}
process::~process() {}

using v8::Exception;
using v8::Isolate;
using v8::String;

pid_t process::openProcess(const char* processName, const char** errorMessage){
    pid_t processPid = -1;


    DIR *dir = opendir("/proc");
    if (dir == NULL) {
        *errorMessage = "unable to open /proc";
        return -1;
    }

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        pid_t pid = strtol(ent->d_name, NULL, 0);
        if (pid == 0) {
            continue;
        }

        char comm_path[4096];
        snprintf(comm_path, sizeof(comm_path), "/proc/%d/comm", pid);

        FILE *f = fopen(comm_path, "r");
        if (f == NULL) {
            continue;
        }

        char comm[4096];
        ssize_t rc = fread(comm, 1, sizeof(comm), f);
        if (rc > 1) {
            comm[rc - 1] = '\0';
            if (strcmp(comm, processName) == 0) {
                processPid = pid;
                fclose(f);
                break;
            }
        }
        fclose(f);
    }

    closedir(dir);


    if (processPid < 0) {
        *errorMessage = "unable to find process";
    }

    return processPid;
}

std::vector<pid_t> process::getProcesses(const char **errorMessage) {
    std::vector<pid_t> pids;

    DIR *dir = opendir("/proc");
    if (dir == NULL) {
        *errorMessage = "unable to open /proc";
        return pids;
    }

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        pid_t pid = strtol(ent->d_name, NULL, 0);
        if (pid == 0) {
            continue;
        }

        pids.push_back(pid);
    }

    closedir(dir);

    return pids;
}

char *process::getProcessPath(pid_t proccessId) {
    char comm[4096];
    char comm_path[4096];
    
    sprintf(comm_path, "/proc/%d/comm", proccessId);

    FILE *f = fopen(comm_path, "r");
    if (f == NULL) {
        return NULL;
    }

    size_t rc = fread(comm, 1, sizeof(comm), f);
    if (rc > 1) {
        comm[rc - 1] = '\0';
    } else {
        fclose(f);
        return NULL;
    }
    fclose(f);

    // Remember to free me please.
    return strdup(comm);
}
