#include <cctype>
#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::secrets;

TEST(PasswordGenerator, DefaultFlagsContainAllContentTypes)
{
	password_generator generator;
	password_content flags = generator.get_content_flags();
	password_content expected =
	    password_content::numeric | password_content::uppercase | password_content::lowercase | password_content::special | password_content::space;
	ASSERT_EQ(flags, expected);
}

TEST(PasswordGenerator, GenerateUsesDefaultLength)
{
	password_generator generator;
	std::string password = generator.generate();
	ASSERT_EQ(password.size(), 16);
}

TEST(PasswordGenerator, GenerateUsesRequestedLength)
{
	password_generator generator;
	std::string password = generator.generate(64);
	ASSERT_EQ(password.size(), 64);
}

TEST(PasswordGenerator, NumericOnlyGeneratesDigits)
{
	password_generator generator;
	generator.set_content_flags(password_content::numeric);
	std::string password = generator.generate(256);
	ASSERT_EQ(password.size(), 256);
	for (char c : password)
	{
		ASSERT_TRUE(std::isdigit(static_cast<unsigned char>(c)) != 0);
	}
}

TEST(PasswordGenerator, LowercaseOnlyGeneratesLowercaseLetters)
{
	password_generator generator;
	generator.set_content_flags(password_content::lowercase);
	std::string password = generator.generate(256);
	ASSERT_EQ(password.size(), 256);
	for (char c : password)
	{
		ASSERT_TRUE(std::islower(static_cast<unsigned char>(c)) != 0);
	}
}

TEST(PasswordGenerator, SpaceOnlyGeneratesSpaces)
{
	password_generator generator;
	generator.set_content_flags(password_content::space);
	std::string password = generator.generate(64);
	ASSERT_EQ(password.size(), 64);
	for (char c : password)
	{
		ASSERT_EQ(c, ' ');
	}
}

TEST(PasswordGenerator, SetContentFlagsUpdatesFlags)
{
	password_generator generator;
	generator.set_content_flags(password_content::uppercase | password_content::special);
	password_content flags = generator.get_content_flags();
	ASSERT_EQ(flags, password_content::uppercase | password_content::special);
}

TEST(PasswordGenerator, CopyConstructorKeepsFlags)
{
	password_generator original{ password_content::numeric | password_content::space };
	password_generator copy{ original };
	ASSERT_EQ(copy.get_content_flags(), password_content::numeric | password_content::space);
}

TEST(PasswordGenerator, CopyAssignmentKeepsFlags)
{
	password_generator source{ password_content::lowercase | password_content::special };
	password_generator destination;
	destination = source;
	ASSERT_EQ(destination.get_content_flags(), password_content::lowercase | password_content::special);
}
