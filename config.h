#ifndef CONFIG_H
#define CONFIG_H

#include <filesystem>
#include <vector>
#include <string>

class Config {
public:
    Config()
        : m_projectFilePath("")
        , m_filename("")
        , m_cppcheck("cppcheck")
        , m_args({})
    {
    }

    /* Load config file.
     * Returns an empty string on success, or an error message
     * on failure. */
    std::string load(const std::filesystem::path &path);

    /* Construct cppcheck command string */
    std::string command() const;

    /* Read command line arguments.
     * Returns an empty string on success, or an error message
     * on failure. */
    std::string parseArgs(int argc, char **argv);

private:
    static std::filesystem::path findConfig(const std::filesystem::path &input_path);

    std::filesystem::path m_projectFilePath = "";
    std::filesystem::path m_filename;
    std::string m_cppcheck;
    std::vector<std::string> m_args;
};

#endif
