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

    std::string text = buffer.data();

    // Parse JSON
    picojson::value data;
    std::string err = picojson::parse(data, text);
    if (!err.empty()) {
        return err;
    }

    if (!data.is<picojson::object>()) 
        return "Invalid config format";

    const picojson::object &obj = data.get<picojson::object>();

    // Read settings
    for (auto [key, value] : obj) {

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

    for (auto arg : m_args)
        cmd += " " + arg;

    if (!m_projectFilePath.empty()) {

        std::string filter = m_filename;
        if (std::strchr(filter.c_str(), ' '))
            filter = "\"" + filter + "\"";

        cmd += " --project=" + m_projectFilePath.string() + " --file-filter=" + filter;

    } else {
        cmd += " " + m_filename;
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

    for (; *argv; ++argv) {
        const char *arg = *argv;
        const char *value;

        if ((value = startsWith(arg, "--config="))) {
            std::string err = load(value);
            if (!err.empty())
                return "Failed to load config file '" + std::string(value) + "': " + err;
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

    return "";
}
