#include <node.h>
#include <vector>
#include <cstring>
#include "module.h"
#include "process.h"
#include "memoryjs.h"

//TODO: Might be ll on other systems
#define KLF "l"

module::Module module::findModule(const char* moduleName, pid_t processId, const char** errorMessage) {
    module::Module result;
    bool found = false;

    char maps_path[4096];
    snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", processId);
    FILE *f = fopen(maps_path, "r");

    if (f == NULL) {
        *errorMessage = "cannot open /proc/.../maps";
        return result;
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

       sscanf(line, "%" KLF "x-%" KLF "x %31s %llx %x:%x %llu", &result.start,
			&result.end, result.permissions, &result.offset, 
            &result.dev_major, &result.dev_minor, &result.inode);
        
        result.pathname = strchr(line,'/');

        found = true;
        break;
    }
    free(line);
    fclose(f);

    if (!found) {
        *errorMessage = "unable to find module";
        printf("did not find module %s for pid %d\n", moduleName, processId);
        return result;
    }

    return result;

}
