#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace desktop::helpers::hash
{
	std::string sha256(const std::filesystem::path& file_path);
	std::string sha256(std::string_view data);
	std::string sha512(const std::filesystem::path& file_path);
	std::string sha512(std::string_view data);
}