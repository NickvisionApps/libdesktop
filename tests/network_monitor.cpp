#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::network;

TEST(NetworkMonitor, IsConnected)
{
	network_monitor monitor;
	ASSERT_EQ(monitor.get_current_state(), network_state::connected_global);
}