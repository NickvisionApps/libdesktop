#pragma once

#include <compare>
#include <string>

namespace desktop::updates
{
	class version
	{
	public:
		version();
		version(int major, int minor, int patch);
		version(int major, int minor, int patch, const std::string& preview);
		version(const std::string& version);
		version(const version&) = default;
		version(version&&) = default;
		int get_major() const;
		int get_minor() const;
		int get_patch() const;
		const std::string& get_preview() const;
		bool is_preview() const;
		bool empty() const;
		const std::string& str() const;
		static bool try_parse(const std::string& s, version& v);
		std::strong_ordering operator<=>(const version& other) const;
		bool operator==(const version& other) const;
		bool operator!=(const version& other) const;
		version& operator=(const version&) = default;
		version& operator=(version&&) = default;

	private:
		void build_str();
		int m_major;
		int m_minor;
		int m_patch;
		std::string m_preview;
		std::string m_str;
	};
}