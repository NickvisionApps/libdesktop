#pragma once

#include <optional>
#include <string>
#include "secrets/secret.h"
#include "services/service.h"

namespace desktop::secrets
{
	class secret_service : public services::service
	{
	public:
		secret_service() = default;
		~secret_service() override = default;
		secret_service(const secret_service&) = delete;
		secret_service(secret_service&&) = delete;
		std::optional<secret> get(const std::string& name) const;
		std::optional<secret> create(const std::string& name);
		bool add(const secret& s);
		bool update(const secret& s);
		bool remove(const std::string& name);
		secret_service& operator=(const secret_service&) = delete;
		secret_service& operator=(secret_service&&) = delete;
	};
}
