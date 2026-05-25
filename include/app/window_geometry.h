#pragma once

#include <nlohmann/json.hpp>

namespace desktop::app
{
	class window_geometry
	{
	public:
		window_geometry();
		~window_geometry() = default;
		window_geometry(int height, int width, bool maximized);
		window_geometry(int height, int width, bool maximized, int x, int y);
		window_geometry(const window_geometry& other) = default;
		window_geometry(window_geometry&& other) noexcept = default;
		int get_height() const;
		void set_height(int height);
		int get_width() const;
		void set_width(int width);
		bool is_maximized() const;
		void set_maximized(bool maximized);
		int get_x() const;
		void set_x(int x);
		int get_y() const;
		void set_y(int y);
		window_geometry& operator=(const window_geometry& other) = default;
		window_geometry& operator=(window_geometry&& other) noexcept = default;

		friend void to_json(nlohmann::json& j, const window_geometry& w)
		{
			j = { { "height", w.get_height() }, { "width", w.get_width() }, { "maximized", w.is_maximized() }, { "x", w.get_x() }, { "y", w.get_y() } };
		}

		friend void from_json(const nlohmann::json& j, window_geometry& w)
		{
			w = window_geometry{ j.at("height").get<int>(), j.at("width").get<int>(), j.at("maximized").get<bool>(), j.at("x").get<int>(),
				                 j.at("y").get<int>() };
		}

	private:
		int m_height;
		int m_width;
		bool m_maximized;
		int m_x;
		int m_y;
	};
}