#ifndef CONFIG_H
#define CONFIG_H

#include <filesystem>
#include <vector>
#include <string>

class Config {
public:
    Config()
        : m_projectFilePath("")
        , m_enable("")
        , m_template("")
        , m_premium("")
        , m_cppcheck("cppcheck")
        , m_filename("")

        // TODO: more settings?
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
    std::filesystem::path m_projectFilePath = "";
    std::string m_enable;
    std::string m_template;
    std::string m_premium;
    std::string m_cppcheck;
    std::string m_filename;

    // TODO: more settings?
};

#endif
