#include "config.h"
#include "picojson.h"

#include <fstream>
#include <vector>
#include <cstring>
#include <cerrno>

std::string Config::load(const std::filesystem::path &path)
{
    // Read config file
    std::ifstream ifs(path);
    if (ifs.fail())
        return std::strerror(errno);

    std::streampos length;
    ifs.seekg(0, std::ios::end);
    length = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    std::vector<char> buffer(length);
    ifs.read(&buffer[0], length);

    if (ifs.fail())
        return std::strerror(errno);

    const std::string text = buffer.data();

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

        if (key == "args") {
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

    cmd += m_cppcheck;

    for (const auto &arg : m_args)
        cmd += " " + arg;

    if (!m_projectFilePath.empty()) {

        std::string filter = m_filename;
        if (std::strchr(filter.c_str(), ' '))
            filter = "\"" + filter + "\"";

        cmd += " --project=" + m_projectFilePath.string() + " --file-filter=" + filter;

    } else {
        cmd += " " + m_filename.string();
    }

    cmd += " 2>&1";

    return cmd;
}

static const char *startsWith(const char *arg, const char *start) {
    if (std::strncmp(arg, start, std::strlen(start)) != 0)
        return NULL;
    return arg + std::strlen(start);
}

std::string Config::parseArgs(int argc, char **argv)
{
    (void) argc;

    ++argv;

    std::filesystem::path configPath = "";

    for (; *argv; ++argv) {
        const char *arg = *argv;
        const char *value;

        if ((value = startsWith(arg, "--config="))) {
            configPath = value;
            continue;
        }

        if (arg[0] == '-')
            return "Invalid option '" + std::string(arg) + "'";

        if (!m_filename.empty())
            return "Multiple filenames provided";

        m_filename = arg;
    }

    if (m_filename.empty())
        return "Missing filename";

    if (configPath.empty())
        configPath = findConfig(m_filename);

    if (configPath.empty())
        return "Failed to find config file";

    const std::string err = load(configPath);
    if (err.empty())
        return "";
    return "Failed to load '" + configPath.string() + "': " + err;
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
