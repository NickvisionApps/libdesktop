#include "filesystem/user_directories.h"
#include <cstdlib>
#include <pwd.h>
#include <unistd.h>

static std::filesystem::path get_home_path()
{
	const char* home = std::getenv("HOME");
	if (home && *home != '\0')
	{
		return std::filesystem::path{ home };
	}
	const struct passwd* pw = getpwuid(getuid());
	if (pw && pw->pw_dir && *pw->pw_dir != '\0')
	{
		return std::filesystem::path{ pw->pw_dir };
	}
	return {};
}

namespace desktop::filesystem
{
	std::filesystem::path user_directories::get_cache()
	{
		std::filesystem::path res{ get_home_path() / "Library" / "Caches" };
		std::filesystem::create_directories(res);
		return res;
	}

	std::filesystem::path user_directories::get_config()
	{
		std::filesystem::path res{ get_home_path() / "Library" / "Application Support" };
		std::filesystem::create_directories(res);
		return res;
	}

	std::filesystem::path user_directories::get_desktop()
	{
		std::filesystem::path res{ get_home_path() / "Desktop" };
		std::filesystem::create_directories(res);
		return res;
	}

	std::filesystem::path user_directories::get_documents()
	{
		std::filesystem::path res{ get_home_path() / "Documents" };
		std::filesystem::create_directories(res);
		return res;
	}

	std::filesystem::path user_directories::get_downloads()
	{
		std::filesystem::path res{ get_home_path() / "Downloads" };
		std::filesystem::create_directories(res);
		return res;
	}

	std::filesystem::path user_directories::get_home()
	{
		return get_home_path();
	}

	std::filesystem::path user_directories::get_local_data()
	{
		return user_directories::get_cache();
	}

	std::filesystem::path user_directories::get_music()
	{
		std::filesystem::path res{ get_home_path() / "Music" };
		std::filesystem::create_directories(res);
		return res;
	}

	std::filesystem::path user_directories::get_pictures()
	{
		std::filesystem::path res{ get_home_path() / "Pictures" };
		std::filesystem::create_directories(res);
		return res;
	}

	std::filesystem::path user_directories::get_templates()
	{
		std::filesystem::path res{ get_home_path() / "Templates" };
		std::filesystem::create_directories(res);
		return res;
	}

	std::filesystem::path user_directories::get_videos()
	{
		std::filesystem::path res{ get_home_path() / "Videos" };
		std::filesystem::create_directories(res);
		return res;
	}
}