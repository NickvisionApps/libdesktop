#pragma once

#include <cstddef>
#include <filesystem>
#include <vector>

namespace desktop::helpers::file
{
	bool extract_archive(const std::filesystem::path& path, const std::filesystem::path& destination);
	std::vector<std::byte> read_bytes(const std::filesystem::path& path);
	bool write_bytes(const std::filesystem::path& path, const std::vector<std::byte>& bytes, bool overwrite = true);
}