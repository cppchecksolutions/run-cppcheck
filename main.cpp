
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <filesystem>

#include <sys/stat.h>  // stat

#include "config.h"

std::ofstream logfile;


/**
 * Execute a shell command and read the output from it. Returns true if command terminated successfully.
 */
static int executeCommand(const std::string& cmd, std::string &output)
{
    output.clear();

#ifdef _WIN32
    FILE* p = _popen(cmd.c_str(), "r");
#else
    FILE *p = popen(cmd.c_str(), "r");
#endif
    if (!p) {
        // TODO: how to provide to caller?
        const int err = errno;
        logfile << "popen() errno " << std::to_string(err) << std::endl;
        return EXIT_FAILURE;
    }
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), p) != nullptr)
        output += buffer;

#ifdef _WIN32
    const int res = _pclose(p);
#else
    const int res = pclose(p);
#endif
    if (res == -1) { // error occurred
        // TODO: how to provide to caller?
        const int err = errno;
        logfile << "pclose() errno " << std::to_string(err) << std::endl;
        return res;
    }
#if !defined(WIN32) && !defined(__MINGW32__)
    if (WIFEXITED(res)) {
        return WEXITSTATUS(res);
    }
    if (WIFSIGNALED(res)) {
        return WTERMSIG(res);
    }
#endif
    return res;
}

int main(int argc, char** argv) {

    const std::string log_path = std::filesystem::current_path() / "log.txt";
    logfile.open(log_path, std::ios_base::app);

    Config config;

    const std::string err = config.parseArgs(argc, argv);
    if (!err.empty()) {
        std::cerr << "error: " << err << std::endl;
        return EXIT_FAILURE;
    }

    const std::string cmd = config.command();

    logfile << "CMD:" << cmd << std::endl;

    std::string output;
    int res = executeCommand(cmd, output);

    std::cerr << output;
    logfile << output;

    return res;
}
