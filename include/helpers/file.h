#pragma once

#include <cstddef>
#include <filesystem>
#include <vector>

namespace desktop::helpers::file
{
	std::vector<std::byte> read_bytes(const std::filesystem::path& path);
	bool write_bytes(const std::filesystem::path& path, const std::vector<std::byte>& bytes, bool overwrite = true);
}