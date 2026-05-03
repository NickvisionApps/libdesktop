#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::secrets;

class SecretsTest : public ::testing::Test
{
protected:
	secret_service m_svc;
	const std::string m_name{ "libdesktop_test" };

	void TearDown() override
	{
		m_svc.remove(m_name);
	}
};

TEST_F(SecretsTest, CreateSecret)
{
	std::optional<secret> s{ m_svc.create(m_name) };
	ASSERT_TRUE(s.has_value());
	EXPECT_FALSE(s->value().empty());
}

TEST_F(SecretsTest, GetSecretMatchesCreatedValue)
{
	std::optional<secret> created{ m_svc.create(m_name) };
	ASSERT_TRUE(created.has_value());
	std::optional<secret> fetched{ m_svc.get(m_name) };
	ASSERT_TRUE(fetched.has_value());
	EXPECT_EQ(fetched->value(), created->value());
}

TEST_F(SecretsTest, UpdateSecret)
{
	ASSERT_TRUE(m_svc.create(m_name).has_value());
	ASSERT_TRUE(m_svc.update({ m_name, "new_value" }));
	std::optional<secret> fetched{ m_svc.get(m_name) };
	ASSERT_TRUE(fetched.has_value());
	EXPECT_EQ(fetched->value(), "new_value");
}

TEST_F(SecretsTest, RemoveSecret)
{
	ASSERT_TRUE(m_svc.create(m_name).has_value());
	ASSERT_TRUE(m_svc.remove(m_name));
	EXPECT_FALSE(m_svc.get(m_name).has_value());
}
