#include <chrono>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <libdesktop.h>
#include <optional>
#include <thread>

using namespace desktop::events;
using namespace desktop::filesystem;

class FolderWatcher : public testing::Test
{
protected:
	void SetUp() override
	{
		m_test_directory = std::filesystem::temp_directory_path() / "libdesktop_folder_watcher_tests";
		std::filesystem::remove_all(m_test_directory);
		std::filesystem::create_directories(m_test_directory);
		m_test_directory = std::filesystem::canonical(m_test_directory);
	}

	void TearDown() override
	{
		std::filesystem::remove_all(m_test_directory);
	}

	std::filesystem::path m_test_directory;
};

TEST_F(FolderWatcher, GetPath)
{
	folder_watcher watcher{ m_test_directory };
	ASSERT_EQ(watcher.get_path(), m_test_directory);
}

TEST_F(FolderWatcher, CreateFileFiresCreatedEvent)
{
	folder_watcher watcher{ m_test_directory };
	std::optional<bool> fired{ std::nullopt };
	watcher.get_created_event().add_handler([&](const folder_watcher&, const folder_watcher_event_args& args)
	{
		if (args.get_name() == "created.txt")
		{
			fired = true;
		}
	});
	std::ofstream file{ (m_test_directory / "created.txt").string() };
	file << "hello";
	while (!fired.has_value())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
	}
	ASSERT_TRUE(fired);
}

TEST_F(FolderWatcher, ModifyFileFiresChangedEvent)
{
	std::filesystem::path file_path{ m_test_directory / "changed.txt" };
	{
		std::ofstream file{ file_path.string() };
		file << "before";
	}
	folder_watcher watcher{ m_test_directory };
	std::optional<bool> fired{ std::nullopt };
	watcher.get_changed_event().add_handler([&](const folder_watcher&, const folder_watcher_event_args& args)
	{
		if (args.get_name() == "changed.txt")
		{
			fired = true;
		}
	});
	{
		std::ofstream file{ file_path.string(), std::ios::app };
		file << "after";
	}
	while (!fired.has_value())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
	}
	ASSERT_TRUE(fired);
}

TEST_F(FolderWatcher, DeleteFileFiresDeletedEvent)
{
	std::filesystem::path file_path{ m_test_directory / "deleted.txt" };
	{
		std::ofstream file{ file_path.string() };
		file << "test";
	}
	folder_watcher watcher{ m_test_directory };
	std::optional<bool> fired{ std::nullopt };
	watcher.get_deleted_event().add_handler([&](const folder_watcher&, const folder_watcher_event_args& args)
	{
		if (args.get_name() == "deleted.txt")
		{
			fired = true;
		}
	});
	std::filesystem::remove(file_path);
	while (!fired.has_value())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
	}
	ASSERT_TRUE(fired);
}

TEST_F(FolderWatcher, RenameFileFiresRenamedEvent)
{
	std::filesystem::path original{ m_test_directory / "before.txt" };
	std::filesystem::path renamed{ m_test_directory / "after.txt" };
	{
		std::ofstream file{ original.string() };
		file << "test";
	}
	folder_watcher watcher{ m_test_directory };
	std::optional<bool> fired{ std::nullopt };
	watcher.get_renamed_event().add_handler([&](const folder_watcher&, const folder_watcher_event_args&)
	{
		fired = true;
	});
	std::filesystem::rename(original, renamed);
	while (!fired.has_value())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
	}
	ASSERT_TRUE(fired);
}

TEST_F(FolderWatcher, EventArgsContainCorrectPath)
{
	folder_watcher watcher{ m_test_directory };
	std::filesystem::path received;
	watcher.get_created_event().add_handler([&](const folder_watcher&, const folder_watcher_event_args& args)
	{
		if (args.get_full_path() == m_test_directory / "path_test.txt")
		{
			received = args.get_full_path();
		}
	});
	std::filesystem::path expected{ m_test_directory / "path_test.txt" };
	{
		std::ofstream file{ expected.string() };
		file << "test";
	}
	while (received.empty())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
	}
	ASSERT_EQ(received, expected);
}

TEST_F(FolderWatcher, EventArgsContainCorrectName)
{
	folder_watcher watcher{ m_test_directory };
	std::string received;
	watcher.get_created_event().add_handler([&](const folder_watcher&, const folder_watcher_event_args& args)
	{
		if (args.get_name() == "name_test.txt")
		{
			received = args.get_name();
		}
	});
	{
		std::ofstream file{ m_test_directory / "name_test.txt" };
		file << "test";
	}
	while (received.empty())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
	}
	ASSERT_EQ(received, "name_test.txt");
}

TEST_F(FolderWatcher, EventArgsContainCorrectFlag)
{
	folder_watcher watcher{ m_test_directory };
	folder_watcher_change_flag received{ folder_watcher_change_flag::any };
	watcher.get_created_event().add_handler([&](const folder_watcher&, const folder_watcher_event_args& args)
	{
		received = args.get_change_flag();
	});
	{
		std::ofstream file{ (m_test_directory / "flag_test.txt").string() };
		file << "test";
	}
	while (received == folder_watcher_change_flag::any)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
	}
	ASSERT_EQ(received, folder_watcher_change_flag::added);
}

TEST_F(FolderWatcher, MultipleHandlersFire)
{
	folder_watcher watcher{ m_test_directory };
	int count{ 0 };
	watcher.get_created_event().add_handler([&](const folder_watcher&, const folder_watcher_event_args&)
	{
		++count;
	});
	watcher.get_created_event().add_handler([&](const folder_watcher&, const folder_watcher_event_args&)
	{
		++count;
	});
	{
		std::ofstream file{ (m_test_directory / "multi.txt").string() };
		file << "test";
	}
	while (count < 2)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
	}
	ASSERT_GE(count, 2);
}

TEST_F(FolderWatcher, WaitForChangeAny)
{
	folder_watcher watcher{ m_test_directory };
	std::thread worker{ [&]()
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		std::ofstream file{ (m_test_directory / "wait_any.txt").string() };
		file << "test";
	} };
	ASSERT_NO_THROW(watcher.wait_for_change(folder_watcher_change_flag::any));
	worker.join();
}

TEST_F(FolderWatcher, WaitForChangeAdded)
{
	folder_watcher watcher{ m_test_directory };
	std::thread worker{ [&]()
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		std::ofstream file{ (m_test_directory / "wait_added.txt").string() };
		file << "test";
	} };

	ASSERT_NO_THROW(watcher.wait_for_change(folder_watcher_change_flag::added));
	worker.join();
}

TEST_F(FolderWatcher, CreatedEventSenderMatchesWatcher)
{
	folder_watcher watcher{ m_test_directory };
	const folder_watcher* sender{ nullptr };
	watcher.get_created_event().add_handler([&](const folder_watcher& s, const folder_watcher_event_args&)
	{
		sender = &s;
	});
	{
		std::ofstream file{ (m_test_directory / "sender.txt").string() };
		file << "test";
	}
	while (sender == nullptr)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
	}
	ASSERT_EQ(sender, &watcher);
}

TEST_F(FolderWatcher, RemovedHandlerDoesNotFire)
{
	folder_watcher watcher{ m_test_directory };
	std::optional<bool> fired{ std::nullopt };
	event_id id = watcher.get_created_event().add_handler([&](const folder_watcher&, const folder_watcher_event_args&)
	{
		fired = true;
	});
	watcher.get_created_event().remove_handler(id);
	{
		std::ofstream file{ (m_test_directory / "removed.txt").string() };
		file << "test";
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(250));
	ASSERT_FALSE(fired);
}