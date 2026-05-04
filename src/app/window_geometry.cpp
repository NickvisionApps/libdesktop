#include "app/window_geometry.h"

namespace desktop::app
{
	window_geometry::window_geometry()
		: m_height{ 900 },
		m_width{ 700 },
		m_maximized{ false },
		m_x{ 10 },
		m_y{ 10 }
	{

	}

	window_geometry::window_geometry(int height, int width, bool maximized)
		: m_height{ height },
		m_width{ width },
		m_maximized{ maximized },
		m_x{ 10 },
		m_y{ 10 }
	{

	}

	window_geometry::window_geometry(int height, int width, bool maximized, int x, int y)
		: m_height{ height },
		m_width{ width },
		m_maximized{ maximized },
		m_x{ x },
		m_y{ y }
	{

	}

	int window_geometry::get_height() const
	{
		return m_height;
	}

	void window_geometry::set_height(int height)
	{
		m_height = height;
	}

	int window_geometry::get_width() const
	{
		return m_width;
	}

	void window_geometry::set_width(int width)
	{
		m_width = width;
	}

	bool window_geometry::is_maximized() const
	{
		return m_maximized;
	}

	void window_geometry::set_maximized(bool maximized)
	{
		m_maximized = maximized;
	}

	int window_geometry::get_x() const
	{
		return m_x;
	}

	void window_geometry::set_x(int x)
	{
		m_x = x;
	}

	int window_geometry::get_y() const
	{
		return m_y;
	}

	void window_geometry::set_y(int y)
	{
		m_y = y;
	}
}