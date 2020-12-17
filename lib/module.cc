#include <node.h>
#include <vector>
#include <cstring>
#include "module.h"
#include "process.h"
#include "memoryjs.h"

uintptr_t module::findModule(const char* moduleName, pid_t processId, const char** errorMessage) {
    uintptr_t address = 0;
    bool found = false;

    char maps_path[4096];
    snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", processId);
    FILE *f = fopen(maps_path, "r");

    if (f == NULL) {
        *errorMessage = "cannot open /proc/.../maps";
        return 0;
    }

    size_t len = 0;
    char *line = NULL;
    ssize_t rc = 0;
    while ((rc = getline(&line, &len, f)) != -1) {
        // Remove trailing \n
        if (rc < 1) {
            continue;
        }
        line[rc-1] = '\0';

        // Search if the moduleName appears *anywhere* in the line
        if (strstr(line, moduleName) == NULL) {
            continue;
        }

        // We only need the first field, which is the starting address
        address = strtoul(line, NULL, 16);
        found = true;
        break;
    }
    free(line);
    fclose(f);

    if (!found) {
        *errorMessage = "unable to find module";
        printf("did not find module %s for pid %d\n", moduleName, processId);
        return 0;
    }

    return address;
} 
