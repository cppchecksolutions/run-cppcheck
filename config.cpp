#include "config.h"
#include "picojson.h"

#include <fstream>
#include <vector>
#include <cstring>
#include <cerrno>

// mkdir and access
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

std::string Config::load(const std::filesystem::path &path)
{
    // Read config file
    std::ifstream ifs(path);
    if (ifs.fail())
        return std::strerror(errno);

    const std::string text(std::istreambuf_iterator<char>(ifs), {});

    // Parse JSON
    picojson::value data;
    const std::string err = picojson::parse(data, text);
    if (!err.empty()) {
        return err;
    }

    if (!data.is<picojson::object>()) 
        return "Invalid config format";

    const picojson::object &obj = data.get<picojson::object>();

    // Read settings
    for (const auto &[key, value] : obj) {

        if (key == "project_file") {
            if (!value.is<std::string>()) {
                return "Invalid value type for '" + key + "'";
            }
            m_projectFilePath = value.get<std::string>();
            continue;
        }

        if (key == "cppcheck") {
            if (!value.is<std::string>()) {
                return "Invalid value type for '" + key + "'";
            }
            m_cppcheck = value.get<std::string>();
            continue;
        }

        if (key == "log_file") {
            if (!value.is<std::string>()) {
                return "Invalid value type for '" + key + "'";
            }
            m_logFilePath = value.get<std::string>();
            continue;
        }

        if (key == "enable_logging") {
            if (!value.is<bool>()) {
                return "Invalid value type for '" + key + "'";
            }
            m_loggingEnabled = value.get<bool>();
            continue;
        }

        if (key == "extra_args") {
            if (!value.is<picojson::array>())
                return "Invalid value type for '" + key + "'";

            for (auto arg : value.get<picojson::array>()) {
                if (!arg.is<std::string>())
                    return "Invalid value type for array element in '" + key + "'";
                m_args.push_back(arg.get<std::string>());
            }
            continue;
        }

        return "Invalid config key '" + key + "'";
    }

    return "";
}

std::string Config::command() const
{
    std::string cmd;

    cmd += "\"" + m_cppcheck + "\"";

    for (const auto &arg : m_args)
        cmd += " \"" + arg + "\"";

    if (!m_projectFilePath.empty()) {
        cmd += " \"--project=" + m_projectFilePath.string() + "\" \"--file-filter=" + m_filename.string() + "\"";
    } else {
        cmd += " \"" + m_filename.string() + "\"";
    }

    cmd += " 2>&1";

#ifdef _WIN32
    cmd = "\"" + cmd + "\"";
#endif

    return cmd;
}

static const char *startsWith(const char *arg, const char *start)
{
    if (std::strncmp(arg, start, std::strlen(start)) != 0)
        return NULL;
    return arg + std::strlen(start);
}

static std::filesystem::path normalizePath(const std::filesystem::path path)
{
    std::filesystem::path result;
    for (auto component : path) {
        if (component.string() == ".")
            continue;
        if (component.string() == "..") {
            result = result.parent_path();
            continue;
        }
        result /= component;
    }
    return result;
}

std::string Config::parseArgs(int argc, char **argv)
{
    (void) argc;

    ++argv;

    for (; *argv; ++argv) {
        const char *arg = *argv;
        const char *value;

        if ((value = startsWith(arg, "--config="))) {
            m_configPath = value;
            continue;
        }

        if (arg[0] == '-') {
            m_args.push_back(arg);
            continue;
        }

        if (!m_filename.empty())
            return "Multiple filenames provided";

        m_filename = arg;
    }

    if (m_filename.empty())
        return "Missing filename";

    if (m_logFilePath.empty()) {
        const std::string error = getDefaultLogFilePath(m_logFilePath);
        if (!error.empty())
            return error;
    }
    if (m_configPath.empty())
        m_configPath = findConfig(m_filename);

    if (m_configPath.empty())
        return "Failed to find 'run-cppcheck-config.json' in any parent directory of analyzed file";

    const std::string err = load(m_configPath);
    if (!err.empty())
        return "Failed to load '" + m_configPath.string() + "': " + err;

    if (!m_projectFilePath.empty() && m_projectFilePath.is_relative())
        m_projectFilePath = m_configPath.parent_path() / m_projectFilePath;

    if (m_filename.is_relative())
        m_filename = normalizePath(std::filesystem::current_path() / m_filename);

    return "";
}

// Find config file by recursively searching parent directories of input file
std::filesystem::path Config::findConfig(const std::filesystem::path &input_path)
{
    auto path = input_path;

    if (path.is_relative())
        path = std::filesystem::current_path() / path;

    do {
        path = path.parent_path();
        const auto config_path = path / "run-cppcheck-config.json";

        if (std::filesystem::exists(config_path))
            return config_path;

    } while (path != path.root_path());

    return "";
}

static std::string mkdirRecursive(const std::filesystem::path &path)
{
    int res;
    std::filesystem::path subpath = "";
    for (auto dir : path) {
        subpath = subpath / dir;
#ifdef _WIN32
        res = _access(subpath.string().c_str(), 0) && _mkdir(subpath.string().c_str());
#else
        res = access(subpath.string().c_str(), F_OK) && mkdir(subpath.string().c_str(), 0777);
#endif
        if (res)
            return "Failed to create '" + subpath.string() + "': " + std::strerror(errno);
    }
    return "";
}

std::string Config::getDefaultLogFilePath(std::filesystem::path &path)
{
    path = "";

#ifdef _WIN32
    const char *localappdata = std::getenv("LOCALAPPDATA");

    if (localappdata) {
        path = localappdata;
    } else {
        return "%LOCALAPPDATA% not set";
    }
#else
    const char *xdg_state_home = std::getenv("XDG_STATE_HOME");

    if (xdg_state_home) {
        path = xdg_state_home;
    } else {
        path = std::string(std::getenv("HOME")) + "/.local/state";
    }
#endif

    path = path / "run-cppcheck";

    const std::string error = mkdirRecursive(path);
    if (!error.empty())
        return error;

    path = path / "log.txt";

    return "";
}
