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
		~host() = default;
		host(const host&) = default;
		host(host&&) noexcept = default;
		std::shared_ptr<services::service_collection>& get_services();
		void run();
		host& operator=(const host&) = default;
		host& operator=(host&&) noexcept = default;

	private:
		std::shared_ptr<services::service_collection> m_services;
	};
}