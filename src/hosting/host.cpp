#include "hosting/host.h"
#include "app/arguments_service.h"
#include "app/configuration_service.h"
#include "app/keyring_service.h"
#include "app/logger.h"
#include "app/translation_service.h"
#include "database/database_service.h"
#include "hosting/lifetime_service.h"
#include "network/http_service.h"
#include "notifications/notification_service.h"
#include "secrets/secret_service.h"
#include "system/power_service.h"
#include "updates/github_update_service.h"

using namespace desktop::app;
using namespace desktop::database;
using namespace desktop::network;
using namespace desktop::notifications;
using namespace desktop::secrets;
using namespace desktop::services;
using namespace desktop::system;
using namespace desktop::updates;

namespace desktop::hosting
{
	host::host(host_options options)
	    : m_options{ std::move(options) },
	      m_services{ std::make_shared<service_collection>() }
	{
		m_services->add<app_info>(m_options.get_app_info());
		m_services->add<arguments_service>(service_scope::singleton, m_options.get_argv());
		m_services->add<configuration_service>(service_scope::singleton);
		m_services->add<database_service>(service_scope::singleton);
		m_services->add<http_service>(service_scope::singleton);
		m_services->add<keyring_service>(service_scope::singleton);
		m_services->add<notification_service>(service_scope::singleton);
		m_services->add<power_service>(service_scope::singleton);
		m_services->add<secret_service>(service_scope::singleton);
		m_services->add<translation_service>(service_scope::singleton);
	}

	std::shared_ptr<service_collection>& host::get_services()
	{
		return m_services;
	}

	std::exception_ptr host::run()
	{
		return m_services->get_required<lifetime_service>()->run(m_options.is_single_instance());
	}

	void host::use_github_updates()
	{
		m_services->add<update_service, github_update_service>(service_scope::singleton);
	}

	void host::use_logging(app::log_type minimum, const std::filesystem::path& path)
	{
		m_services->add<logger>(service_scope::singleton, minimum, path);
	}
}