#include "updates/version.h"
#include <stdexcept>

namespace desktop::updates
{
	version::version()
	    : m_major{ 0 },
	      m_minor{ 0 },
	      m_patch{ 0 }
	{
		build_str();
	}

	version::version(int major, int minor, int patch)
	    : m_major{ major },
	      m_minor{ minor },
	      m_patch{ patch }
	{
		build_str();
	}

	version::version(int major, int minor, int patch, const std::string& preview)
	    : m_major{ major },
	      m_minor{ minor },
	      m_patch{ patch },
	      m_preview{ preview }
	{
		if (preview.empty())
		{
			throw std::invalid_argument("preview label must not be empty; use the three-argument constructor for stable versions.");
		}
		build_str();
	}

	version::version(const std::string& s)
	    : m_major{ 0 },
	      m_minor{ 0 },
	      m_patch{ 0 }
	{
		std::string v{ s };
		if (!v.empty() && v[0] == 'v')
		{
			v.erase(0, 1);
		}
		std::size_t dash{ v.find('-') };
		if (dash != std::string::npos)
		{
			m_preview = v.substr(dash + 1);
			v = v.substr(0, dash);
		}
		std::size_t dot1{ v.find('.') };
		if (dot1 == std::string::npos)
		{
			throw std::invalid_argument("Invalid version format.");
		}
		std::size_t dot2{ v.find('.', dot1 + 1) };
		if (dot2 == std::string::npos)
		{
			throw std::invalid_argument("Invalid version format.");
		}
		m_major = std::stoi(v.substr(0, dot1));
		m_minor = std::stoi(v.substr(dot1 + 1, dot2 - dot1 - 1));
		m_patch = std::stoi(v.substr(dot2 + 1));
		build_str();
	}

	int version::get_major() const
	{
		return m_major;
	}

	int version::get_minor() const
	{
		return m_minor;
	}

	int version::get_patch() const
	{
		return m_patch;
	}

	const std::string& version::get_preview() const
	{
		return m_preview;
	}

	bool version::is_preview() const
	{
		return !m_preview.empty();
	}

	bool version::empty() const
	{
		return m_major == 0 && m_minor == 0 && m_patch == 0 && m_preview.empty();
	}

	const std::string& version::str() const
	{
		return m_str;
	}

	std::optional<version> version::parse(const std::string& s)
	{
		try
		{
			return version{ s };
		}
		catch (...)
		{
			return std::nullopt;
		}
	}

	std::strong_ordering version::operator<=>(const version& other) const
	{
		if (auto cmp{ m_major <=> other.m_major }; cmp != 0)
		{
			return cmp;
		}
		if (auto cmp{ m_minor <=> other.m_minor }; cmp != 0)
		{
			return cmp;
		}
		if (auto cmp{ m_patch <=> other.m_patch }; cmp != 0)
		{
			return cmp;
		}
		if (m_preview.empty() && other.m_preview.empty())
		{
			return std::strong_ordering::equal;
		}
		if (m_preview.empty())
		{
			return std::strong_ordering::greater;
		}
		if (other.m_preview.empty())
		{
			return std::strong_ordering::less;
		}
		return m_preview <=> other.m_preview;
	}

	bool version::operator==(const version& other) const
	{
		return m_major == other.m_major && m_minor == other.m_minor && m_patch == other.m_patch && m_preview == other.m_preview;
	}

	bool version::operator!=(const version& other) const
	{
		return !(operator==(other));
	}

	void version::build_str()
	{
		m_str = std::to_string(m_major) + "." + std::to_string(m_minor) + "." + std::to_string(m_patch);
		if (!m_preview.empty())
		{
			m_str += "-" + m_preview;
		}
	}

	version::operator bool() const
	{
		return !empty();
	}
}