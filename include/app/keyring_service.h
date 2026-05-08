#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <vector>
#include "database/database_service.h"
#include "secrets/credential.h"
#include "services/service.h"

namespace desktop::app
{
	class keyring_service : public services::service
	{
	public:
		using dependencies = std::tuple<database::database_service>;
		keyring_service(std::shared_ptr<database::database_service> db);
		~keyring_service() override = default;
		keyring_service(const keyring_service&) = delete;
		keyring_service(keyring_service&&) = delete;
		bool add_credential(const secrets::credential& credential);
		bool delete_credential(const secrets::credential& credential);
		const std::vector<secrets::credential>& get_all_credentials();
		bool update_credential(const secrets::credential& credential);
		keyring_service& operator=(const keyring_service&) = delete;
		keyring_service& operator=(keyring_service&&) = delete;

	private:
		void ensure_table();
		mutable std::mutex m_mutex;
		std::shared_ptr<database::database_service> m_db;
		std::atomic<bool> m_table_ensured;
		std::vector<secrets::credential> m_credentials;
	};
}