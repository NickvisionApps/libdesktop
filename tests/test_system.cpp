#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::system;

class PowerServiceTest : public ::testing::Test
{
protected:
	power_service m_svc;

	void TearDown() override
	{
		m_svc.allow_suspend();
	}
};

TEST(SystemTest, ProcessExitsWithCodeZero)
{
#ifdef _WIN32
	process proc{ "cmd", { "/c", "echo Hello, World!" } };
#else
	process proc{ "/bin/echo", { "Hello, World!" } };
#endif
	ASSERT_TRUE(proc.start());
	EXPECT_EQ(proc.wait_for_exit(), 0);
}

TEST(SystemTest, ProcessStandardOutputNotEmpty)
{
#ifdef _WIN32
	process proc{ "cmd", { "/c", "echo Hello, World!" } };
#else
	process proc{ "/bin/echo", { "Hello, World!" } };
#endif
	ASSERT_TRUE(proc.start());
	proc.wait_for_exit();
	EXPECT_FALSE(proc.get_standard_output().empty());
}

TEST_F(PowerServiceTest, InitialStateAllowsSuspend)
{
	EXPECT_FALSE(m_svc.is_suspended());
}

TEST_F(PowerServiceTest, PreventSuspend)
{
	ASSERT_TRUE(m_svc.prevent_suspend());
	EXPECT_TRUE(m_svc.is_suspended());
}

TEST_F(PowerServiceTest, AllowSuspend)
{
	ASSERT_TRUE(m_svc.prevent_suspend());
	ASSERT_TRUE(m_svc.allow_suspend());
	EXPECT_FALSE(m_svc.is_suspended());
}
