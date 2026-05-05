#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::secrets;

class Secrets_Test : public ::testing::Test
{
protected:
	secret_service m_svc;
	const std::string m_name{ "libdesktop_test" };

	void TearDown() override
	{
		m_svc.remove(m_name);
	}
};

TEST_F(Secrets_Test, PasswordContent_andAssign)
{
	password_content flags{ password_content::numeric | password_content::uppercase };
	flags &= password_content::numeric;
	EXPECT_EQ(flags, password_content::numeric);
}

TEST_F(Secrets_Test, PasswordContent_bitwiseAnd)
{
	password_content flags{ password_content::numeric | password_content::uppercase };
	EXPECT_EQ(flags & password_content::numeric, password_content::numeric);
}

TEST_F(Secrets_Test, PasswordContent_bitwiseNot)
{
	password_content flags{ password_content::numeric | password_content::uppercase | password_content::lowercase | password_content::special |
		                    password_content::space };
	password_content result{ ~flags };
	EXPECT_EQ(result & flags, static_cast<password_content>(0));
}

TEST_F(Secrets_Test, PasswordContent_bitwiseOr)
{
	password_content result{ password_content::numeric | password_content::uppercase };
	EXPECT_EQ(result & password_content::numeric, password_content::numeric);
	EXPECT_EQ(result & password_content::uppercase, password_content::uppercase);
}

TEST_F(Secrets_Test, PasswordContent_bitwiseXor)
{
	password_content flags{ password_content::numeric | password_content::uppercase };
	EXPECT_EQ(flags ^ password_content::numeric, password_content::uppercase);
}

TEST_F(Secrets_Test, PasswordContent_orAssign)
{
	password_content flags{ password_content::numeric };
	flags |= password_content::uppercase;
	EXPECT_EQ(flags & password_content::numeric, password_content::numeric);
	EXPECT_EQ(flags & password_content::uppercase, password_content::uppercase);
}

TEST_F(Secrets_Test, PasswordContent_xorAssign)
{
	password_content flags{ password_content::numeric | password_content::uppercase };
	flags ^= password_content::numeric;
	EXPECT_EQ(flags, password_content::uppercase);
}

TEST_F(Secrets_Test, PasswordGenerator_contentFlags)
{
	password_generator gen{ password_content::numeric };
	EXPECT_EQ(gen.get_content_flags(), password_content::numeric);
}

TEST_F(Secrets_Test, PasswordGenerator_next)
{
	password_generator gen;
	EXPECT_EQ(gen.generate(16).length(), 16u);
}

TEST_F(Secrets_Test, PasswordGenerator_nextUniqueValues)
{
	password_generator gen;
	EXPECT_NE(gen.generate(), gen.generate());
}

TEST_F(Secrets_Test, PasswordGenerator_setContentFlags)
{
	password_generator gen;
	gen.set_content_flags(password_content::numeric);
	EXPECT_EQ(gen.get_content_flags(), password_content::numeric);
}

TEST_F(Secrets_Test, Secret_empty)
{
	EXPECT_TRUE(secret("", "").empty());
	EXPECT_FALSE(secret("key", "value").empty());
}

TEST_F(Secrets_Test, Secret_name)
{
	secret s{ "myKey", "myValue" };
	EXPECT_EQ(s.get_name(), "myKey");
}

TEST_F(Secrets_Test, Secret_operatorBool)
{
	EXPECT_FALSE(static_cast<bool>(secret("", "")));
	EXPECT_TRUE(static_cast<bool>(secret("key", "value")));
}

TEST_F(Secrets_Test, Secret_value)
{
	secret s{ "myKey", "myValue" };
	EXPECT_EQ(s.get_value(), "myValue");
}

TEST_F(Secrets_Test, SecretService_add)
{
	secret s{ m_name, "preset_value" };
	ASSERT_TRUE(m_svc.add(s));
	std::optional<secret> fetched{ m_svc.get(m_name) };
	ASSERT_TRUE(fetched.has_value());
	EXPECT_EQ(fetched->get_value(), "preset_value");
}

TEST_F(Secrets_Test, SecretService_create)
{
	std::optional<secret> s{ m_svc.create(m_name) };
	ASSERT_TRUE(s.has_value());
	EXPECT_FALSE(s->get_value().empty());
}

TEST_F(Secrets_Test, SecretService_get)
{
	std::optional<secret> created{ m_svc.create(m_name) };
	ASSERT_TRUE(created.has_value());
	std::optional<secret> fetched{ m_svc.get(m_name) };
	ASSERT_TRUE(fetched.has_value());
	EXPECT_EQ(fetched->get_value(), created->get_value());
}

TEST_F(Secrets_Test, SecretService_remove)
{
	ASSERT_TRUE(m_svc.create(m_name).has_value());
	ASSERT_TRUE(m_svc.remove(m_name));
	EXPECT_FALSE(m_svc.get(m_name).has_value());
}

TEST_F(Secrets_Test, SecretService_update)
{
	ASSERT_TRUE(m_svc.create(m_name).has_value());
	ASSERT_TRUE(m_svc.update({ m_name, "new_value" }));
	std::optional<secret> fetched{ m_svc.get(m_name) };
	ASSERT_TRUE(fetched.has_value());
	EXPECT_EQ(fetched->get_value(), "new_value");
}
