#include "helpers/file.h"
#include <fstream>

namespace desktop::helpers
{
	std::vector<std::byte> file::read_bytes(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path))
		{
			return {};
		}
		std::ifstream file{ path, std::ios::binary };
		if (file.good())
		{
			std::vector<std::byte> bytes{ std::filesystem::file_size(path) };
			file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
			return bytes;
		}
		return {};
	}

	bool file::write_bytes(const std::filesystem::path& path, const std::vector<std::byte>& bytes, bool overwrite)
	{
		if (std::filesystem::exists(path) && !overwrite)
		{
			return false;
		}
		std::ofstream file{ path, std::ios::binary | std::ios::trunc };
		if (file.good())
		{
			file.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
			return true;
		}
		return false;
	}
}