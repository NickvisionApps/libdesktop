#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::app;
using namespace desktop::network;
using namespace desktop::system;
using namespace desktop::updates;

TEST(GithubUpdateService, ParabolicStable)
{
	if (environment::test_variable("GITHUB_ACTIONS"))
	{
		GTEST_SKIP() << "Unable to use GitHub API in CI/CD";
	}
	std::shared_ptr<app_info> parabolic_info{ std::make_shared<app_info>("org.nickvision.tubeconverter", "Parabolic", "Parabolic", false) };
	std::shared_ptr<http_service> http{ std::make_shared<http_service>() };
	ASSERT_TRUE(parabolic_info);
	ASSERT_TRUE(http);
	parabolic_info->set_source_url("https://github.com/NickvisionApps/Parabolic");
	github_update_service update{ parabolic_info, http };
	std::optional<version> latest{ update.get_latest_version(false) };
	ASSERT_TRUE(latest.has_value());
	ASSERT_TRUE(latest.value() >= version(2026, 5, 0));
}

TEST(GithubUpdateService, ParabolicPreview)
{
	if (environment::test_variable("GITHUB_ACTIONS"))
	{
		GTEST_SKIP() << "Unable to use GitHub API in CI/CD";
	}
	std::shared_ptr<app_info> parabolic_info{ std::make_shared<app_info>("org.nickvision.tubeconverter", "Parabolic", "Parabolic", false) };
	std::shared_ptr<http_service> http{ std::make_shared<http_service>() };
	ASSERT_TRUE(parabolic_info);
	ASSERT_TRUE(http);
	parabolic_info->set_source_url("https://github.com/NickvisionApps/Parabolic");
	github_update_service update{ parabolic_info, http };
	std::optional<version> latest{ update.get_latest_version(true) };
	ASSERT_TRUE(latest.has_value());
	ASSERT_TRUE(latest.value() >= version(2026, 4, 1, "beta1"));
}

TEST(GithubUpdateService, Ytdlp)
{
	if (environment::test_variable("GITHUB_ACTIONS"))
	{
		GTEST_SKIP() << "Unable to use GitHub API in CI/CD";
	}
	std::shared_ptr<http_service> http{ std::make_shared<http_service>() };
	ASSERT_TRUE(http);
	github_update_service update{ "yt-dlp", "yt-dlp", http };
	std::optional<version> latest{ update.get_latest_version(false) };
	ASSERT_TRUE(latest.has_value());
	ASSERT_TRUE(latest.value() >= version(2026, 3, 17));
}

TEST(GithubUpdateService, YtdlpPreview)
{
	if (environment::test_variable("GITHUB_ACTIONS"))
	{
		GTEST_SKIP() << "Unable to use GitHub API in CI/CD";
	}
	std::shared_ptr<http_service> http{ std::make_shared<http_service>() };
	ASSERT_TRUE(http);
	github_update_service update{ "yt-dlp", "yt-dlp-nightly-builds", http };
	std::optional<version> latest{ update.get_latest_version(false) };
	ASSERT_TRUE(latest.has_value());
	ASSERT_TRUE(latest.value() >= version(2026, 5, 24, 234402));
}

TEST(GithubUpdateService, Deno)
{
	if (environment::test_variable("GITHUB_ACTIONS"))
	{
		GTEST_SKIP() << "Unable to use GitHub API in CI/CD";
	}
	std::shared_ptr<http_service> http{ std::make_shared<http_service>() };
	ASSERT_TRUE(http);
	github_update_service update{ "denoland", "deno", http };
	std::optional<version> latest{ update.get_latest_version(false) };
	ASSERT_TRUE(latest.has_value());
	ASSERT_TRUE(latest.value() >= version(2, 8, 0));
}