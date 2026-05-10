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

using namespace desktop::app;
using namespace desktop::database;
using namespace desktop::network;
using namespace desktop::notifications;
using namespace desktop::secrets;
using namespace desktop::services;
using namespace desktop::system;

namespace desktop::hosting
{
	host::host(const host_options& options)
	    : m_services{ std::make_shared<service_collection>() }
	{
		m_services->add_service<app_info>(options.get_app_info());
		m_services->add_service<arguments_service>(service_scope::singleton, options.get_argv());
		m_services->add_service<configuration_service>(service_scope::singleton);
		m_services->add_service<database_service>(service_scope::singleton);
		m_services->add_service<http_service>(service_scope::singleton);
		m_services->add_service<keyring_service>(service_scope::singleton);
		m_services->add_service<logger>(service_scope::singleton, options.get_log_path());
		m_services->add_service<notification_service>(service_scope::singleton);
		m_services->add_service<power_service>(service_scope::singleton);
		m_services->add_service<secret_service>(service_scope::singleton);
		m_services->add_service<translation_service>(service_scope::singleton);
	}

	std::shared_ptr<service_collection>& host::get_services()
	{
		return m_services;
	}

	void host::run()
	{
		m_services->get_required_service<lifetime_service>()->run();
	}
}