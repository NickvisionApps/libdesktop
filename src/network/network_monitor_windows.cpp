#include "network/network_monitor.h"
#include <windows.h>
#include <atlbase.h>
#include <netlistmgr.h>
#include <stdexcept>

using namespace desktop::events;

namespace desktop::network
{
	class network_monitor::impl : public INetworkListManagerEvents
	{
	public:
		impl(network_monitor& owner)
		    : m_owner{ owner },
		      m_net_list_manager{ nullptr },
		      m_connection_point{ nullptr }
		{
			m_handles_com = CoInitializeEx(nullptr, COINIT_MULTITHREADED) == S_OK;
			CComPtr<IConnectionPointContainer> connection_point_container{ nullptr };
			CComPtr<IUnknown> sink{ nullptr };
			if (CoCreateInstance(CLSID_NetworkListManager, nullptr, CLSCTX_ALL, __uuidof(INetworkListManager),
			                     reinterpret_cast<LPVOID*>(&m_net_list_manager)) != S_OK)
			{
				throw std::runtime_error("Unable to create network list manager.");
			}
			if (m_net_list_manager->QueryInterface(IID_PPV_ARGS(&connection_point_container)) != S_OK)
			{
				throw std::runtime_error("Unable to create connection point container.");
			}
			if (connection_point_container->FindConnectionPoint(IID_INetworkListManagerEvents, &m_connection_point) != S_OK)
			{
				throw std::runtime_error("Unable to find connection point.");
			}
			if (QueryInterface(IID_IUnknown, reinterpret_cast<void**>(&sink)) != S_OK)
			{
				throw std::runtime_error("Unable to get sink pointer.");
			}
			if (m_connection_point->Advise(sink, &m_cookie) != S_OK)
			{
				throw std::runtime_error("Unable to get advice connection point.");
			}
			check_connection_state(false);
		}

		virtual ~impl() noexcept
		{
			if (m_connection_point)
			{
				m_connection_point->Unadvise(m_cookie);
			}
			if (m_handles_com)
			{
				CoUninitialize();
			}
		}

		impl(const impl&) = delete;

		impl(impl&&) noexcept = delete;

		network_state get_current_state() const
		{
			std::scoped_lock lock{ m_mutex };
			return m_current_state;
		}

		ULONG STDMETHODCALLTYPE AddRef() override
		{
			return ++m_ref_count;
		}

		ULONG STDMETHODCALLTYPE Release() override
		{
			return --m_ref_count;
		}

		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppv_obj) override
		{
			if (!ppv_obj)
			{
				return E_POINTER;
			}
			*ppv_obj = nullptr;
			if (riid == IID_IUnknown || riid == IID_INetworkListManagerEvents)
			{
				AddRef();
				*ppv_obj = reinterpret_cast<void*>(this);
				return S_OK;
			}
			return E_NOINTERFACE;
		}

		HRESULT STDMETHODCALLTYPE ConnectivityChanged(NLM_CONNECTIVITY connectivity) override
		{
			check_connection_state(true);
			return S_OK;
		}

		HRESULT STDMETHODCALLTYPE NetworkConnectionPropertyChanged(GUID id, NLM_CONNECTION_PROPERTY_CHANGE change)
		{
			return S_OK;
		}

		impl& operator=(const impl&) = delete;

		impl& operator=(impl&&) noexcept = delete;

	private:
		void check_connection_state(bool event) noexcept
		{
			network_state new_state{ network_state::disconnected };
			NLM_CONNECTIVITY connectivity{ NLM_CONNECTIVITY_DISCONNECTED };
			if (m_net_list_manager->GetConnectivity(&connectivity) == S_OK)
			{
				if (connectivity == NLM_CONNECTIVITY_DISCONNECTED)
				{
					new_state = network_state::disconnected;
				}
				else if (connectivity & NLM_CONNECTIVITY_IPV4_INTERNET || connectivity & NLM_CONNECTIVITY_IPV6_INTERNET)
				{
					new_state = network_state::connected_global;
				}
				else
				{
					new_state = network_state::connected_local;
				}
			}
			std::unique_lock<std::mutex> lock{ m_mutex };
			if (m_current_state != new_state)
			{
				m_current_state = new_state;
				lock.unlock();
				if (event)
				{
					m_owner.m_state_changed_event.invoke(m_owner, { new_state });
				}
			}
		}
		mutable std::mutex m_mutex;
		network_monitor& m_owner;
		network_state m_current_state{ network_state::disconnected };
		CComPtr<INetworkListManager> m_net_list_manager;
		CComPtr<IConnectionPoint> m_connection_point;
		DWORD m_cookie{ 0 };
		ULONG m_ref_count{ 1 };
		bool m_handles_com{ false };
	};

	network_monitor::network_monitor()
	    : m_impl{ std::make_unique<impl>(*this) }
	{
	}

	network_monitor::~network_monitor() = default;

	const event<network_monitor, param_event_args<network_state>>& network_monitor::get_state_changed_event() const
	{
		return m_state_changed_event;
	}

	network_state network_monitor::get_current_state() const
	{
		return m_impl->get_current_state();
	}
}