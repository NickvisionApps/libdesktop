#include "helpers/file.h"
#include <archive.h>
#include <archive_entry.h>
#include <fstream>

namespace desktop::helpers
{
	bool extract_archive(const std::filesystem::path& path, const std::filesystem::path& destination)
	{
		archive* read{ archive_read_new() };
		archive* write{ archive_write_disk_new() };
		archive_entry* entry{ nullptr };
		archive_read_support_filter_all(read);
		archive_read_support_format_all(read);
		archive_write_disk_set_options(write, ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS);
		archive_write_disk_set_standard_lookup(write);
		if (archive_read_open_filename(read, path.string().c_str(), 10240) != ARCHIVE_OK)
		{
			archive_read_free(read);
			archive_write_free(write);
			return false;
		}
		int r{ -1 };
		while ((r = archive_read_next_header(read, &entry)) == ARCHIVE_OK)
		{
			std::filesystem::path output{ destination / archive_entry_pathname(entry) };
			archive_entry_set_pathname(entry, output.string().c_str());
			if (archive_write_header(write, entry) != ARCHIVE_OK)
			{
				archive_read_close(read);
				archive_read_free(read);
				archive_write_close(write);
				archive_write_free(write);
				return false;
			}
			if (archive_entry_size(entry) > 0)
			{
				const void* buff{ nullptr };
				size_t size{ 0 };
				la_int64_t offset{ 0 };
				while (archive_read_data_block(read, &buff, &size, &offset) == ARCHIVE_OK)
				{
					archive_write_data_block(write, buff, size, offset);
				}
			}
		}
		archive_read_close(read);
		archive_read_free(read);
		archive_write_close(write);
		archive_write_free(write);
		return r == ARCHIVE_EOF;
	}

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