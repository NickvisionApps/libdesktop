#pragma once

#include <compare>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace desktop::updates
{
	class version
	{
	public:
		version();
		~version() = default;
		version(int major, int minor, int patch);
		version(int major, int minor, int patch, const std::string& preview);
		version(const std::string& s);
		version(const version&) = default;
		version(version&&) = default;
		int get_major() const;
		int get_minor() const;
		int get_patch() const;
		const std::string& get_preview() const;
		bool is_preview() const;
		bool empty() const;
		const std::string& str() const;
		static std::optional<version> parse(const std::string& s);
		std::strong_ordering operator<=>(const version& other) const;
		bool operator==(const version& other) const;
		bool operator!=(const version& other) const;
		version& operator=(const version&) = default;
		version& operator=(version&&) = default;
		operator bool() const;

		friend void to_json(nlohmann::json& j, const version& v)
		{
			j = { { "major", v.get_major() }, { "minor", v.get_minor() }, { "patch", v.get_patch() }, { "preview", v.get_preview() } };
		}

		friend void from_json(const nlohmann::json& j, version& v)
		{
			v = version{ j.at("major").get<int>(), j.at("minor").get<int>(), j.at("patch").get<int>(), j.value("preview", std::string{}) };
		}

	private:
		void build_str();
		int m_major;
		int m_minor;
		int m_patch;
		std::string m_preview;
		std::string m_str;
	};
}