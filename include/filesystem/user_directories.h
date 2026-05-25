#pragma once

#include <filesystem>

namespace desktop::filesystem::user_directories
{
	[[nodiscard]] std::filesystem::path get_cache();
	[[nodiscard]] std::filesystem::path get_config();
	[[nodiscard]] std::filesystem::path get_desktop();
	[[nodiscard]] std::filesystem::path get_documents();
	[[nodiscard]] std::filesystem::path get_downloads();
	[[nodiscard]] std::filesystem::path get_home();
	[[nodiscard]] std::filesystem::path get_local_data();
	[[nodiscard]] std::filesystem::path get_music();
	[[nodiscard]] std::filesystem::path get_pictures();
	[[nodiscard]] std::filesystem::path get_templates();
	[[nodiscard]] std::filesystem::path get_videos();
}