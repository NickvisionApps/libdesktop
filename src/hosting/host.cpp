#include "hosting/host.h"
#include "app/arguments_service.h"
#include "app/logger.h"
#include "hosting/lifetime_service.h"

using namespace desktop::services;

namespace desktop::hosting
{
	host::host(const host_options& options)
		: m_services{ std::make_shared<service_collection>()}
	{
		m_services->add_service<app::arguments_service>(service_scope::singleton, options.get_argc(), options.get_argv());
		m_services->add_service<app::logger>(service_scope::singleton, options.get_log_path());
	}

	std::shared_ptr<service_collection> host::services()
	{
		return m_services;
	}

	void host::run()
	{
		std::shared_ptr<lifetime_service> lifetime = m_services->get_required_service<lifetime_service>();
		lifetime->run();
	}
}