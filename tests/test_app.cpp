#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::app;

TEST(AppTest, LoggerDebug)
{
	logger log;
	EXPECT_NO_THROW(log.debug("This is a debug message.", __FILE__, __LINE__));
}

TEST(AppTest, LoggerInfo)
{
	logger log;
	EXPECT_NO_THROW(log.info("This is an info message.", __FILE__, __LINE__));
}

TEST(AppTest, LoggerWarn)
{
	logger log;
	EXPECT_NO_THROW(log.warn("This is a warning message.", __FILE__, __LINE__));
}

TEST(AppTest, LoggerError)
{
	logger log;
	EXPECT_NO_THROW(log.error("This is an error message.", __FILE__, __LINE__));
}

TEST(AppTest, LoggerCritical)
{
	logger log;
	EXPECT_NO_THROW(log.critical("This is a critical message.", __FILE__, __LINE__));
}

TEST(AppTest, LoggerWithFilePath)
{
	logger log{ "test_app.log" };
	EXPECT_NO_THROW(log.info("This is a logged-to-file message.", __FILE__, __LINE__));
}
