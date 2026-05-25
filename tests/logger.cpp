#include <gtest/gtest.h>
#include <libdesktop.h>
#include <memory>

using namespace desktop::app;

TEST(Logger, DebugNoFile)
{
	logger log{ log_type::debug };
	ASSERT_TRUE(log.debug("Test"));
	ASSERT_TRUE(log.info("Test"));
	ASSERT_TRUE(log.warn("Test"));
	ASSERT_TRUE(log.error("Test"));
	ASSERT_TRUE(log.critical("Test"));
}

TEST(Logger, InfoNoFile)
{
	logger log{ log_type::info };
	ASSERT_FALSE(log.debug("Test"));
	ASSERT_TRUE(log.info("Test"));
	ASSERT_TRUE(log.warn("Test"));
	ASSERT_TRUE(log.error("Test"));
	ASSERT_TRUE(log.critical("Test"));
}

TEST(Logger, WarnNoFile)
{
	logger log{ log_type::warn };
	ASSERT_FALSE(log.debug("Test"));
	ASSERT_FALSE(log.info("Test"));
	ASSERT_TRUE(log.warn("Test"));
	ASSERT_TRUE(log.error("Test"));
	ASSERT_TRUE(log.critical("Test"));
}

TEST(Logger, ErrorNoFile)
{
	logger log{ log_type::error };
	ASSERT_FALSE(log.debug("Test"));
	ASSERT_FALSE(log.info("Test"));
	ASSERT_FALSE(log.warn("Test"));
	ASSERT_TRUE(log.error("Test"));
	ASSERT_TRUE(log.critical("Test"));
}

TEST(Logger, CriticalNoFile)
{
	logger log{ log_type::critical };
	ASSERT_FALSE(log.debug("Test"));
	ASSERT_FALSE(log.info("Test"));
	ASSERT_FALSE(log.warn("Test"));
	ASSERT_FALSE(log.error("Test"));
	ASSERT_TRUE(log.critical("Test"));
}

TEST(Logger, DebugFile)
{
	if (std::filesystem::exists("debug.log"))
	{
		std::filesystem::remove("debug.log");
	}
	std::shared_ptr<logger> log{ std::make_shared<logger>(log_type::debug, "debug.log") };
	ASSERT_TRUE(log->debug("Test"));
	ASSERT_TRUE(log->info("Test"));
	ASSERT_TRUE(log->warn("Test"));
	ASSERT_TRUE(log->error("Test"));
	ASSERT_TRUE(log->critical("Test"));
	ASSERT_TRUE(std::filesystem::exists("debug.log"));
	log.reset();
	std::filesystem::remove("debug.log");
	ASSERT_FALSE(std::filesystem::exists("debug.log"));
}

TEST(Logger, InfoFile)
{
	if (std::filesystem::exists("info.log"))
	{
		std::filesystem::remove("info.log");
	}
	std::shared_ptr<logger> log{ std::make_shared<logger>(log_type::info, "info.log") };
	ASSERT_FALSE(log->debug("Test"));
	ASSERT_TRUE(log->info("Test"));
	ASSERT_TRUE(log->warn("Test"));
	ASSERT_TRUE(log->error("Test"));
	ASSERT_TRUE(log->critical("Test"));
	ASSERT_TRUE(std::filesystem::exists("info.log"));
	log.reset();
	std::filesystem::remove("info.log");
	ASSERT_FALSE(std::filesystem::exists("info.log"));
}

TEST(Logger, WarnFile)
{
	if (std::filesystem::exists("warn.log"))
	{
		std::filesystem::remove("warn.log");
	}
	std::shared_ptr<logger> log{ std::make_shared<logger>(log_type::warn, "warn.log") };
	ASSERT_FALSE(log->debug("Test"));
	ASSERT_FALSE(log->info("Test"));
	ASSERT_TRUE(log->warn("Test"));
	ASSERT_TRUE(log->error("Test"));
	ASSERT_TRUE(log->critical("Test"));
	ASSERT_TRUE(std::filesystem::exists("warn.log"));
	log.reset();
	std::filesystem::remove("warn.log");
	ASSERT_FALSE(std::filesystem::exists("warn.log"));
}

TEST(Logger, ErrorFile)
{
	if (std::filesystem::exists("error.log"))
	{
		std::filesystem::remove("error.log");
	}
	std::shared_ptr<logger> log{ std::make_shared<logger>(log_type::error, "error.log") };
	ASSERT_FALSE(log->debug("Test"));
	ASSERT_FALSE(log->info("Test"));
	ASSERT_FALSE(log->warn("Test"));
	ASSERT_TRUE(log->error("Test"));
	ASSERT_TRUE(log->critical("Test"));
	ASSERT_TRUE(std::filesystem::exists("error.log"));
	log.reset();
	std::filesystem::remove("error.log");
	ASSERT_FALSE(std::filesystem::exists("error.log"));
}

TEST(Logger, CriticalFile)
{
	if (std::filesystem::exists("critical.log"))
	{
		std::filesystem::remove("critical.log");
	}
	std::shared_ptr<logger> log{ std::make_shared<logger>(log_type::critical, "critical.log") };
	ASSERT_FALSE(log->debug("Test"));
	ASSERT_FALSE(log->info("Test"));
	ASSERT_FALSE(log->warn("Test"));
	ASSERT_FALSE(log->error("Test"));
	ASSERT_TRUE(log->critical("Test"));
	ASSERT_TRUE(std::filesystem::exists("critical.log"));
	log.reset();
	std::filesystem::remove("critical.log");
	ASSERT_FALSE(std::filesystem::exists("critical.log"));
}