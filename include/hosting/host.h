#pragma once

#include <memory>
#include "host_options.h"
#include "services/service_collection.h"

namespace desktop::hosting
{
	class host
	{
	public:
		host(const host_options& options);
		std::shared_ptr<services::service_collection> services();
		void run();

	private:
		std::shared_ptr<services::service_collection> m_services;
	};
}