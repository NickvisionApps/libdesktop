#pragma once

#include <cstdint>

namespace desktop::network
{
	class download_progress
	{
	public:
		download_progress(std::int64_t received, std::int64_t total);
		~download_progress() = default;
		download_progress(const download_progress&) = default;
		download_progress(download_progress&&) = default;
		std::int64_t get_received() const;
		std::int64_t get_total() const;
		download_progress& operator=(const download_progress&) = default;
		download_progress& operator=(download_progress&&) = default;

	private:
		std::int64_t m_received;
		std::int64_t m_total;
	};
}