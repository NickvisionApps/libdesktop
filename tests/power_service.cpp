#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::system;

TEST(PowerService, InitialState)
{
	power_service service;
	ASSERT_FALSE(service.is_suspended());
}

TEST(PowerService, PreventSuspend)
{
#ifdef __linux__
	if (environment::test_variable("GITHUB_ACTIONS"))
	{
		GTEST_SKIP() << "Unable to use power management API in CI/CD";
	}
#endif

	power_service service;
	ASSERT_TRUE(service.prevent_suspend());
	ASSERT_TRUE(service.is_suspended());
}

TEST(PowerService, AllowSuspend)
{
#ifdef __linux__
	if (environment::test_variable("GITHUB_ACTIONS"))
	{
		GTEST_SKIP() << "Unable to use power management API in CI/CD";
	}
#endif
	power_service service;
	ASSERT_TRUE(service.prevent_suspend());
	ASSERT_TRUE(service.is_suspended());
	ASSERT_TRUE(service.allow_suspend());
	ASSERT_FALSE(service.is_suspended());
}

TEST(PowerService, AllowSuspendWithoutPrevent)
{
	power_service service;
	ASSERT_TRUE(service.allow_suspend());
	ASSERT_FALSE(service.is_suspended());
}

TEST(PowerService, PreventSuspendTwice)
{
#ifdef __linux__
	if (environment::test_variable("GITHUB_ACTIONS"))
	{
		GTEST_SKIP() << "Unable to use power management API in CI/CD";
	}
#endif
	power_service service;
	ASSERT_TRUE(service.prevent_suspend());
	ASSERT_TRUE(service.prevent_suspend());
	ASSERT_TRUE(service.is_suspended());
}

TEST(PowerService, AllowSuspendTwice)
{
#ifdef __linux__
	if (environment::test_variable("GITHUB_ACTIONS"))
	{
		GTEST_SKIP() << "Unable to use power management API in CI/CD";
	}
#endif
	power_service service;
	ASSERT_TRUE(service.prevent_suspend());
	ASSERT_TRUE(service.allow_suspend());
	ASSERT_TRUE(service.allow_suspend());
	ASSERT_FALSE(service.is_suspended());
}

TEST(PowerService, PreventAllowPrevent)
{
#ifdef __linux__
	if (environment::test_variable("GITHUB_ACTIONS"))
	{
		GTEST_SKIP() << "Unable to use power management API in CI/CD";
	}
#endif
	power_service service;
	ASSERT_TRUE(service.prevent_suspend());
	ASSERT_TRUE(service.is_suspended());
	ASSERT_TRUE(service.allow_suspend());
	ASSERT_FALSE(service.is_suspended());
	ASSERT_TRUE(service.prevent_suspend());
	ASSERT_TRUE(service.is_suspended());
}

TEST(PowerService, DestructorReleasesSuspend)
{
#ifdef __linux__
	if (environment::test_variable("GITHUB_ACTIONS"))
	{
		GTEST_SKIP() << "Unable to use power management API in CI/CD";
	}
#endif
	{
		power_service service;
		ASSERT_TRUE(service.prevent_suspend());
		ASSERT_TRUE(service.is_suspended());
	}
	SUCCEED();
}

TEST(PowerService, MultipleInstances)
{
#ifdef __linux__
	if (environment::test_variable("GITHUB_ACTIONS"))
	{
		GTEST_SKIP() << "Unable to use power management API in CI/CD";
	}
#endif
	power_service first;
	power_service second;
	ASSERT_TRUE(first.prevent_suspend());
	ASSERT_TRUE(second.prevent_suspend());
	ASSERT_TRUE(first.is_suspended());
	ASSERT_TRUE(second.is_suspended());
	ASSERT_TRUE(first.allow_suspend());
	ASSERT_TRUE(second.allow_suspend());
	ASSERT_FALSE(first.is_suspended());
	ASSERT_FALSE(second.is_suspended());
}