#include "network/download_progress.h"

namespace desktop::network
{
	download_progress::download_progress(std::int64_t received, std::int64_t total)
	    : m_received{ received },
	      m_total{ total }
	{
	}

	std::int64_t download_progress::get_received() const
	{
		return m_received;
	}

	std::int64_t download_progress::get_total() const
	{
		return m_total;
	}
}