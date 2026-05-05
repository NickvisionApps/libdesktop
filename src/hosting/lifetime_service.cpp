#include <format>
#include "hosting/lifetime_service.h"
#ifdef _WIN32
#include <windows.h>
#endif

using namespace desktop::app;
using namespace desktop::services;

namespace desktop::hosting
{
	lifetime_service::lifetime_service(std::shared_ptr<logger> logger, bool graphical)
	    : m_logger{ logger },
	      m_graphical{ graphical }
	{
	}

	lifetime_service::~lifetime_service()
	{
		stop();
		if (m_thread.joinable())
		{
			m_thread.join();
		}
	}

	const std::stop_source& lifetime_service::get_stop_source() const
	{
		return m_stop_source;
	}

	std::chrono::seconds lifetime_service::get_uptime() const
	{
		return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - m_start_time);
	}

	void lifetime_service::run()
	{
		m_logger->debug("Starting application.", __FILE__, __LINE__);
		m_start_time = std::chrono::steady_clock::now();
		std::stop_callback callback{ m_stop_source.get_token(), [this]()
		{
			m_logger->debug("Stop application requested.", __FILE__, __LINE__);
			on_stop_requested();
		} };
		if (!m_graphical)
		{
			m_thread = std::thread(&lifetime_service::startup_and_run, this);
		}
		else
		{
#ifdef _WIN32
			m_thread = std::thread([this]()
			{
				HRESULT hr{ CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED) };
				if (SUCCEEDED(hr))
				{
					startup_and_run();
					CoUninitialize();
				}
			});
#elif defined(__APPLE__)
			startup_and_run();
#else
			m_thread = std::thread(&lifetime_service::startup_and_run, this);
#endif
		}
		if (m_thread.joinable())
		{
			m_thread.join();
		}
		shutdown();
	}

	void lifetime_service::stop()
	{
		m_stop_source.request_stop();
	}

	void lifetime_service::startup_and_run()
	{
		try
		{
			on_startup_and_run();
		}
		catch (const std::exception& e)
		{
			m_logger->critical(e.what(), __FILE__, __LINE__);
			stop();
		}
		catch (...)
		{
			m_logger->critical("An unknown exception occurred.", __FILE__, __LINE__);
			stop();
		}
	}

	void lifetime_service::shutdown()
	{
		m_logger->debug("Shutting down application.", __FILE__, __LINE__);
		on_shutdown();
		m_logger->info(std::format("Application ran for {} seconds.", get_uptime().count()), __FILE__, __LINE__);
	}
}