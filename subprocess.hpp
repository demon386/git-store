#include <stdio.h>

namespace subprocess {
using ExitStatusT = int;

ExitStatusT run_cmd(const std::string& cmd, bool require_stdout=true) {
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        return -1;
    }
    constexpr int buffer_size = 128;
    char buffer[buffer_size];
    while (!feof(pipe)) {
        if (fgets(buffer, buffer_size, pipe) != nullptr) {
            if (require_stdout) {
                printf("%s", buffer);
            }
        }
    }
    return pclose(pipe);
}
} // namespace: subprocess

