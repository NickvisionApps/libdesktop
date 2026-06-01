#include <chrono>
#include <gtest/gtest.h>
#include <libdesktop.h>
#include <memory>
#include <optional>
#include <string>
#include <vector>

using namespace desktop::secrets;

class SecretService : public testing::Test
{
protected:
	void SetUp() override
	{
		m_service = std::make_shared<secret_service>();
	}

	void TearDown() override
	{
		for (std::string& name : m_names_to_cleanup)
		{
			m_service->remove(name);
		}
		m_names_to_cleanup.clear();
		m_service.reset();
	}

	std::shared_ptr<secret_service> m_service;
	std::vector<std::string> m_names_to_cleanup;
};

TEST_F(SecretService, GetMissingSecretReturnsNullopt)
{
	long long ticks = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	std::string name = "libdesktop.test.secret." + std::to_string(ticks);
	m_names_to_cleanup.push_back(name);
	std::optional<secret> value = m_service->get(name);
	ASSERT_FALSE(value.has_value());
}

TEST_F(SecretService, AddEmptySecretValueReturnsFalse)
{
	long long ticks = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	std::string name = "libdesktop.test.secret." + std::to_string(ticks);
	m_names_to_cleanup.push_back(name);
	secret value{ name, "" };
	ASSERT_FALSE(m_service->add(value));
}

TEST_F(SecretService, UpdateEmptySecretValueReturnsFalse)
{
	long long ticks = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	std::string name = "libdesktop.test.secret." + std::to_string(ticks);
	m_names_to_cleanup.push_back(name);
	secret value{ name, "" };
	ASSERT_FALSE(m_service->update(value));
}

TEST_F(SecretService, RemoveMissingSecretReturnsFalse)
{
	long long ticks = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	std::string name = "libdesktop.test.secret." + std::to_string(ticks);
	m_names_to_cleanup.push_back(name);
	ASSERT_FALSE(m_service->remove(name));
}

TEST_F(SecretService, CreateReturnsGeneratedValueWhenBackendAvailable)
{
	long long ticks = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	std::string name = "libdesktop.test.secret." + std::to_string(ticks);
	m_names_to_cleanup.push_back(name);
	std::optional<secret> value = m_service->create(name);
	if (!value.has_value())
	{
		GTEST_SKIP() << "Secret backend not available";
	}
	ASSERT_EQ(value->get_name(), name);
	ASSERT_FALSE(value->get_value().empty());
	ASSERT_EQ(value->get_value().size(), 64);
}

TEST_F(SecretService, AddGetUpdateRemoveRoundTripWhenBackendAvailable)
{
	long long ticks = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	std::string name = "libdesktop.test.secret." + std::to_string(ticks);
	m_names_to_cleanup.push_back(name);
	secret initial{ name, "value_1" };
	if (!m_service->add(initial))
	{
		GTEST_SKIP() << "Secret backend not available";
	}
	std::optional<secret> loaded_initial = m_service->get(name);
	ASSERT_TRUE(loaded_initial.has_value());
	ASSERT_EQ(loaded_initial->get_name(), name);
	ASSERT_EQ(loaded_initial->get_value(), "value_1");
	secret updated{ name, "value_2" };
	ASSERT_TRUE(m_service->update(updated));
	std::optional<secret> loaded_updated = m_service->get(name);
	ASSERT_TRUE(loaded_updated.has_value());
	ASSERT_EQ(loaded_updated->get_value(), "value_2");
	ASSERT_TRUE(m_service->remove(name));
	std::optional<secret> removed_value = m_service->get(name);
	ASSERT_FALSE(removed_value.has_value());
}
