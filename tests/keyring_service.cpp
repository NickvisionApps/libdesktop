#include <filesystem>
#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::app;
using namespace desktop::database;
using namespace desktop::secrets;

class KeyringService : public testing::Test
{
protected:
	void SetUp() override
	{
		std::filesystem::path db_path{ desktop::filesystem::user_directories::get_config() / "KeyringServiceTests" / "app.db" };
		std::error_code ec;
		std::filesystem::remove(db_path, ec);
		std::filesystem::remove(db_path.parent_path(), ec);
		std::shared_ptr<app_info> info{ std::make_shared<app_info>("libdesktop.test.keyring", "KeyringServiceTests", "KeyringServiceTests", false) };
		std::shared_ptr<secret_service> secrets{ std::make_shared<secret_service>() };
		m_db = std::make_shared<database_service>(info, secrets);
		m_service = std::make_shared<keyring_service>(m_db);
	}

	void TearDown() override
	{
		m_service.reset();
		m_db.reset();
		std::filesystem::path db_path{ desktop::filesystem::user_directories::get_config() / "KeyringServiceTests" / "app.db" };
		std::error_code ec;
		std::filesystem::remove(db_path, ec);
		std::filesystem::remove(db_path.parent_path(), ec);
	}

	std::shared_ptr<database_service> m_db;
	std::shared_ptr<keyring_service> m_service;
};

TEST_F(KeyringService, GetAllCredentialsEmptyInitially)
{
	ASSERT_TRUE(m_service->get_all_credentials().empty());
}

TEST_F(KeyringService, AddCredential)
{
	credential c{ "github", "nick", "pass", "https://github.com" };
	ASSERT_TRUE(m_service->add_credential(c));
	std::vector<credential> all = m_service->get_all_credentials();
	ASSERT_EQ(all.size(), 1);
	ASSERT_EQ(all[0].get_name(), "github");
	ASSERT_EQ(all[0].get_username(), "nick");
	ASSERT_EQ(all[0].get_password(), "pass");
	ASSERT_EQ(all[0].get_url(), "https://github.com");
}

TEST_F(KeyringService, AddCredentialDuplicateNameFails)
{
	ASSERT_TRUE(m_service->add_credential({ "github", "first", "first_pw", "https://first.example" }));
	ASSERT_FALSE(m_service->add_credential({ "github", "second", "second_pw", "https://second.example" }));
	ASSERT_EQ(m_service->get_all_credentials().size(), 1);
}

TEST_F(KeyringService, DeleteCredential)
{
	credential c{ "service", "user", "pw", "https://example.com" };
	ASSERT_TRUE(m_service->add_credential(c));
	ASSERT_TRUE(m_service->delete_credential(c));
	ASSERT_TRUE(m_service->get_all_credentials().empty());
}

TEST_F(KeyringService, DeleteCredentialMissingReturnsFalse)
{
	ASSERT_FALSE(m_service->delete_credential({ "missing", "u", "p", "https://none.example" }));
}

TEST_F(KeyringService, UpdateCredential)
{
	ASSERT_TRUE(m_service->add_credential({ "mail", "u1", "p1", "https://mail.example" }));
	ASSERT_TRUE(m_service->update_credential({ "mail", "u2", "p2", "https://mail2.example" }));
	std::vector<credential> all = m_service->get_all_credentials();
	ASSERT_EQ(all.size(), 1);
	ASSERT_EQ(all[0].get_name(), "mail");
	ASSERT_EQ(all[0].get_username(), "u2");
	ASSERT_EQ(all[0].get_password(), "p2");
	ASSERT_EQ(all[0].get_url(), "https://mail2.example");
}

TEST_F(KeyringService, UpdateCredentialMissingReturnsFalse)
{
	ASSERT_FALSE(m_service->update_credential({ "missing", "u", "p", "https://none.example" }));
}

TEST_F(KeyringService, ExistingRowsLoadIntoFreshService)
{
	ASSERT_TRUE(m_service->add_credential({ "persisted", "user", "pw", "https://persist.example" }));
	m_service.reset();
	std::shared_ptr<keyring_service> second_service{ std::make_shared<keyring_service>(m_db) };
	std::vector<credential> all = second_service->get_all_credentials();
	ASSERT_EQ(all.size(), 1);
	ASSERT_EQ(all[0].get_name(), "persisted");
}
