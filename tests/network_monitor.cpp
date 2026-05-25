#include <filesystem>
#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::network;
using namespace desktop::system;

TEST(NetworkMonitor, IsConnected)
{
#ifdef __linux__
	if (environment::test_variable("GITHUB_ACTIONS"))
	{
		GTEST_SKIP() << "No D-Bus system bus available in CI";
	}
#endif
	network_monitor monitor;
	if (environment::get_deployment_mode() == deployment_mode::wsl)
	{
		ASSERT_EQ(monitor.get_current_state(), network_state::connected_local);
	}
	else
	{
		ASSERT_EQ(monitor.get_current_state(), network_state::connected_global);
	}
}