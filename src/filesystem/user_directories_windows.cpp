#include "filesystem/user_directories.h"
#include <shlobj.h>

static std::filesystem::path get_known_folder(const KNOWNFOLDERID& id)
{
	PWSTR path{ nullptr };
	if (SUCCEEDED(SHGetKnownFolderPath(id, KF_FLAG_DEFAULT, nullptr, &path)))
	{
		std::filesystem::path result{ path };
		CoTaskMemFree(path);
		return result;
	}
	return {};
}

namespace desktop::filesystem
{
	std::filesystem::path user_directories::get_cache()
	{
		std::filesystem::path res{ get_known_folder(FOLDERID_LocalAppData) };
		if (!res.empty())
		{
			res /= "Temp";
		}
		std::filesystem::create_directories(res);
		return res;
	}

	std::filesystem::path user_directories::get_config()
	{
		std::filesystem::path res{ get_known_folder(FOLDERID_RoamingAppData) };
		std::filesystem::create_directories(res);
		return res;
	}

	std::filesystem::path user_directories::get_desktop()
	{
		std::filesystem::path res{ get_known_folder(FOLDERID_Desktop) };
		std::filesystem::create_directories(res);
		return res;
	}

	std::filesystem::path user_directories::get_documents()
	{
		std::filesystem::path res{ get_known_folder(FOLDERID_Documents) };
		std::filesystem::create_directories(res);
		return res;
	}

	std::filesystem::path user_directories::get_downloads()
	{
		std::filesystem::path res{ get_known_folder(FOLDERID_Downloads) };
		if (res.empty())
		{
			res = get_home() / "Downloads";
		}
		return res;
	}

	std::filesystem::path user_directories::get_home()
	{
		return get_known_folder(FOLDERID_Profile);
	}

	std::filesystem::path user_directories::get_local_data()
	{
		std::filesystem::path res{ get_known_folder(FOLDERID_LocalAppData) };
		std::filesystem::create_directories(res);
		return res;
	}

	std::filesystem::path user_directories::get_music()
	{
		std::filesystem::path res{ get_known_folder(FOLDERID_Music) };
		std::filesystem::create_directories(res);
		return res;
	}

	std::filesystem::path user_directories::get_pictures()
	{
		std::filesystem::path res{ get_known_folder(FOLDERID_Pictures) };
		std::filesystem::create_directories(res);
		return res;
	}

	std::filesystem::path user_directories::get_templates()
	{
		std::filesystem::path res{ get_known_folder(FOLDERID_Templates) };
		std::filesystem::create_directories(res);
		return res;
	}

	std::filesystem::path user_directories::get_videos()
	{
		std::filesystem::path res{ get_known_folder(FOLDERID_Videos) };
		std::filesystem::create_directories(res);
		return res;
	}
}