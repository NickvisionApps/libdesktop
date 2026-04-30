#include "filesystem/user_directories.h"
#include <cstdlib>
#include <fstream>
#include <string>
#include <pwd.h>
#include <unistd.h>

static std::string trim(const std::string& s)
{
    const std::size_t start{ s.find_first_not_of(" \t\r\n") };
    if(start == std::string::npos)
    {
        return {};
    }
    const std::size_t end{ s.find_last_not_of(" \t\r\n") };
    return s.substr(start, end - start + 1);
}

static std::filesystem::path get_home_path()
{
    const char* home = std::getenv("HOME");
    if(home && *home != '\0')
    {
        return std::filesystem::path{ home };
    }
    const struct passwd* pw = getpwuid(getuid());
    if(pw && pw->pw_dir && *pw->pw_dir != '\0')
    {
        return std::filesystem::path{ pw->pw_dir };
    }
    return {};
}

static std::filesystem::path get_xdg_user_dir(const std::string& name)
{
    if(name.empty())
    {
        return {};
    }
    const char* path = std::getenv(name.c_str())
    if(path && *path != '\0')
    {
        return std::filesystem::path{ path };
    }
    std::filesystem::path dirs_path{ user_directories::get_config() / "user-dirs.dirs" };
    if(!std::filesystem::exists(dirs_path))
    {
        return {};
    }
    std::ifstream file{ dirs_path };
    std::string line;
    std::string home_str{ get_home_path().string() };
    while(std::getline(file, line))
    {
        std::string trimmed{ trim(line) };
        if(trimmed.empty() || trimmed.front() == '#')
        {
            continue;
        }
        std::size_t eq{ trimmed.find('=') };
        if(eq == std::string::npos)
        {
            continue;
        }
        if(trim(trimmed.substr(0, eq)) != name)
        {
            continue;
        }
        std::string value{ trim(trimmed.substr(eq + 1)) };
        while(!value.empty() && value.front() == '"')
        {
            value.erase(value.begin());
        }
        while(!value.empty() && value.back() == '"')
        {
            value.pop_back();
        }
        std::size_t pos{ 0 };
        while((pos = value.find("$HOME", pos)) != std::string::npos)
        {
            value.replace(pos, 5, home_str);
            pos += home_str.size();
        }
        if(!value.empty())
        {
            return std::filesystem::path{ value };
        }
        break;
    }
    return {};
}

namespace desktop::filesystem
{
    std::filesystem::path user_directories::get_cache()
    {
        std::filesystem::path res;
        if(const char* dir = std::getenv("XDG_CACHE_HOME"))
        {
            if(*dir != '\0')
            {
                res = std::filesystem::path{ dir };
            }
        }
        if(res.empty())
        {
            res = get_home_path() / ".cache";
        }
        std::filesystem::create_directories(res);
        return res;
    }

    std::filesystem::path user_directories::get_config()
    {
        std::filesystem::path res;
        if(const char* dir = std::getenv("XDG_CONFIG_HOME"))
        {
            if(*dir != '\0')
            {
                res = std::filesystem::path{ dir };
            }
        }
        if(res.empty())
        {
            res = get_home_path() / ".config";
        }
        std::filesystem::create_directories(res);
        return res;
    }

    std::filesystem::path user_directories::get_desktop()
    {
        std::filesystem::path res{ get_xdg_user_dir("XDG_DESKTOP_DIR") };
        if(res.empty())
        {
            res = get_home_path() / "Desktop";
        }
        std::filesystem::create_directories(res);
        return res;
    }

    std::filesystem::path user_directories::get_documents()
    {
        std::filesystem::path res{ get_xdg_user_dir("XDG_DOCUMENTS_DIR") };
        if(res.empty())
        {
            res = get_home_path() / "Documents";
        }
		std::filesystem::create_directories(res);
        return res;
    }

    std::filesystem::path user_directories::get_downloads()
    {
        std::filesystem::path res{ get_xdg_user_dir("XDG_DOWNLOAD_DIR") };
        if(res.empty())
        {
            res = get_home_path() / "Downloads";
        }
		std::filesystem::create_directories(res);
        return res;
    }

    std::filesystem::path user_directories::get_home()
    {
        return get_home_path();
    }

    std::filesystem::path user_directories::get_local_data()
    {
        std::filesystem::path res;
        if(const char* dir = std::getenv("XDG_DATA_HOME"))
        {
            if(*dir != '\0')
            {
                res = std::filesystem::path{ dir };
            }
        }
        if(res.empty())
        {
            res = get_home_path() / ".local" / "share";
        }
        std::filesystem::create_directories(res);
        return res;
    }

    std::filesystem::path user_directories::get_music()
    {
        std::filesystem::path res{ get_xdg_user_dir("XDG_MUSIC_DIR") };
        if(res.empty())
        {
            res = get_home_path() / "Music";
        }
        std::filesystem::create_directories(res);
        return res;
    }

    std::filesystem::path user_directories::get_pictures()
    {
        std::filesystem::path res{ get_xdg_user_dir("XDG_PICTURES_DIR") };
        if(res.empty())
        {
            res = get_home_path() / "Pictures";
        }
        std::filesystem::create_directories(res);
        return res;
    }

    std::filesystem::path user_directories::get_templates()
    {
        std::filesystem::path res{ get_xdg_user_dir("XDG_TEMPLATES_DIR") };
        if(res.empty())
        {
            res = get_home_path() / "Templates";
        }
        std::filesystem::create_directories(res);
        return res;
    }

    std::filesystem::path user_directories::get_videos()
    {
        std::filesystem::path res{ get_xdg_user_dir("XDG_VIDEOS_DIR") };
        if(res.empty())
        {
            res = get_home_path() / "Videos";
        }
        std::filesystem::create_directories(res);
        return res;
    }
}