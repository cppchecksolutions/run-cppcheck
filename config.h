#ifndef CONFIG_H
#define CONFIG_H

#include <filesystem>
#include <vector>
#include <string>

class Config {
public:
    Config()
        : m_projectFilePath("")
        , m_logFilePath("")
        , m_configPath("")
        , m_loggingEnabled(true)
        , m_cppcheck("cppcheck")
        , m_filename("")
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

    const std::filesystem::path logFilePath() const
    {
        return m_logFilePath;
    }

    const std::filesystem::path configPath() const
    {
        return m_configPath;
    }

private:
    static std::filesystem::path findFile(const std::filesystem::path &input_path, const std::string &filename);
    static std::string getDefaultLogFilePath(std::filesystem::path &path);
    std::string matchFilenameFromCompileCommand();

    std::filesystem::path m_projectFilePath;
    std::filesystem::path m_logFilePath;
    std::filesystem::path m_configPath;
    bool m_loggingEnabled;
    std::string m_cppcheck;
    std::filesystem::path m_filename;
    std::vector<std::string> m_args;
};

#endif
