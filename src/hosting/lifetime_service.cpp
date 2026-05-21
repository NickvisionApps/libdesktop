#include "hosting/lifetime_service.h"
#ifdef _WIN32
#include <windows.h>
#endif

using namespace desktop::app;

namespace desktop::hosting
{
	lifetime_service::lifetime_service(const std::shared_ptr<app_info>& info)
	    : m_graphical{ info->is_graphical() },
	      m_start_time{ std::chrono::steady_clock::now() },
	      m_exception{ nullptr }
	{
	}

	lifetime_service::~lifetime_service()
	{
		request_stop();
		if (m_worker.joinable())
		{
			m_worker.join();
		}
	}

	std::chrono::seconds lifetime_service::get_uptime() const
	{
		return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - m_start_time);
	}

	void lifetime_service::request_restart() noexcept
	{
		try
		{
			std::unique_lock lock{ m_mutex };
			std::stop_source src{ m_stop_source };
			m_should_restart = true;
			lock.unlock();
			src.request_stop();
		}
		catch (...)
		{
		}
	}

	void lifetime_service::request_stop() noexcept
	{
		try
		{
			std::unique_lock lock{ m_mutex };
			std::stop_source src{ m_stop_source };
			m_should_restart = false;
			lock.unlock();
			src.request_stop();
		}
		catch (...)
		{
		}
	}

	std::exception_ptr lifetime_service::run()
	{
#ifdef __APPLE__
		if (m_graphical)
		{
			while (true)
			{
				run_once();
				std::scoped_lock lock{ m_mutex };
				if (!m_should_restart || m_exception)
				{
					break;
				}
				m_should_restart = false;
			}
			on_shutdown();
			return m_exception;
		}
#endif
		m_worker = std::thread([this]()
		{
#ifdef _WIN32
			HRESULT hr = CoInitializeEx(nullptr, m_graphical ? COINIT_APARTMENTTHREADED : COINIT_MULTITHREADED);
			if (FAILED(hr))
			{
				m_exception = std::make_exception_ptr(std::runtime_error("Failed to initialize COM"));
				return;
			}
#endif
			while (true)
			{
				run_once();
				std::scoped_lock lock{ m_mutex };
				if (!m_should_restart || m_exception)
				{
					break;
				}
				m_should_restart = false;
			}
			on_shutdown();
#ifdef _WIN32
			CoUninitialize();
#endif
		});
		m_worker.join();
		return m_exception;
	}

	void lifetime_service::run_once()
	{
		std::unique_lock lock{ m_mutex };
		m_start_time = std::chrono::steady_clock::now();
		m_stop_source = std::stop_source{};
		std::stop_callback callback{ m_stop_source.get_token(), [this]()
		{
			on_stop_requested();
		} };
		lock.unlock();
		try
		{
			on_startup_and_run();
		}
		catch (...)
		{
			lock.lock();
			std::stop_source src{ m_stop_source };
			m_exception = std::current_exception();
			m_should_restart = false;
			lock.unlock();
			src.request_stop();
		}
	}
}