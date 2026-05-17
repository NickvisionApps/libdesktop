#include "notifications/notification_service.h"
#include <windows.h>
#include <filesystem>
#include <shellapi.h>
#include <stdexcept>
#include <thread>
#include <vector>
#include "system/environment.h"

using namespace desktop::system;

static constexpr UINT WM_TRAYICON{ WM_APP + 1 };
static constexpr UINT TRAYICON_ID{ 1001 };

namespace desktop::notifications
{
	notification_service::notification_service(std::shared_ptr<app::app_info> app_info, std::shared_ptr<app::translation_service> translation_service)
	    : m_app_info{ std::move(app_info) },
	      m_translation_service{ std::move(translation_service) }
	{
		std::string class_name{ m_app_info->get_id() + "_notification" };
		WNDCLASSA wc{};
		wc.lpfnWndProc = +[](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT
		{
			if (msg == WM_CREATE)
			{
				CREATESTRUCTA* cs{ reinterpret_cast<CREATESTRUCTA*>(lParam) };
				SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
				return 0;
			}
			if (msg == WM_TRAYICON)
			{
				if (lParam == NIN_BALLOONUSERCLICK)
				{
					std::filesystem::path* open_path{ reinterpret_cast<std::filesystem::path*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA)) };
					if (open_path && std::filesystem::exists(*open_path))
					{
						ShellExecuteA(hwnd, "open", open_path->string().c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
					}
					PostQuitMessage(0);
				}
				else if (lParam == NIN_BALLOONTIMEOUT || lParam == NIN_BALLOONHIDE)
				{
					PostQuitMessage(0);
				}
			}
			else if (msg == WM_DESTROY)
			{
				PostQuitMessage(0);
			}
			return DefWindowProcA(hwnd, msg, wParam, lParam);
		};
		wc.hInstance = GetModuleHandleA(nullptr);
		wc.lpszClassName = class_name.c_str();
		if (RegisterClassA(&wc) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
		{
			throw std::runtime_error{ "Failed to register window class for notifications." };
		}
	}

	notification_service::~notification_service()
	{
		std::string class_name{ m_app_info->get_id() + "_notification" };
		UnregisterClassA(class_name.c_str(), GetModuleHandleA(nullptr));
	}

	const events::event<notification_service, app_notification_sent_event_args>& notification_service::get_app_notification_sent_event() const
	{
		return m_app_notification_sent_event;
	}

	void notification_service::send(const app_notification& notification)
	{
		m_app_notification_sent_event.invoke(*this, { notification });
	}

	void notification_service::send(const shell_notification& notification)
	{
		std::thread worker{ [app_info = m_app_info, notification]()
		{
			std::filesystem::path open_path{ notification.get_action() == "open" ? notification.get_action_parameter() : "" };
			std::string class_name{ app_info->get_id() + "_notification" };
			HWND hwnd{ CreateWindowExA(0, class_name.c_str(), app_info->get_short_name().c_str(), 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr,
				                       GetModuleHandleA(nullptr), &open_path) };
			if (!hwnd)
			{
				return;
			}
			HICON small_icon{ nullptr };
			ExtractIconExA(environment::get_executable_path().string().c_str(), 0, nullptr, &small_icon, 1);
			NOTIFYICONDATAA nid{};
			nid.cbSize = sizeof(NOTIFYICONDATAA);
			nid.hWnd = hwnd;
			nid.uID = TRAYICON_ID;
			nid.uFlags = NIF_INFO | NIF_ICON | NIF_TIP | NIF_MESSAGE;
			nid.uCallbackMessage = WM_TRAYICON;
			nid.hIcon = small_icon ? small_icon : LoadIconA(nullptr, IDI_APPLICATION);
			switch (notification.get_severity())
			{
			case notification_severity::error:
				nid.dwInfoFlags = NIIF_ERROR;
				break;
			case notification_severity::warning:
				nid.dwInfoFlags = NIIF_WARNING;
				break;
			default:
				nid.dwInfoFlags = NIIF_INFO;
				break;
			}
			strcpy_s(nid.szTip, app_info->get_short_name().c_str());
			strcpy_s(nid.szInfoTitle, notification.get_title().c_str());
			strcpy_s(nid.szInfo, notification.get_message().c_str());
			Shell_NotifyIconA(NIM_ADD, &nid);
			MSG msg;
			while (GetMessageA(&msg, nullptr, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessageA(&msg);
			}
			Shell_NotifyIconA(NIM_DELETE, &nid);
			if (small_icon)
			{
				DestroyIcon(small_icon);
			}
			DestroyWindow(hwnd);
		} };
		worker.detach();
	}
}