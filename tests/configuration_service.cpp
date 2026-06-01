#include <filesystem>
#include <gtest/gtest.h>
#include <libdesktop.h>
#include <utility>

using namespace desktop::app;
using namespace desktop::database;
using namespace desktop::secrets;

enum class TestEnum
{
	alpha = 0,
	beta = 1,
	gamma = 2
};

class TestConfig
{
public:
	TestConfig() = default;
	TestConfig(int x, std::string label, bool active)
	    : m_x{ x },
	      m_label{ std::move(label) },
	      m_active{ active }
	{
	}

	int get_x() const
	{
		return m_x;
	}

	void set_x(int x)
	{
		m_x = x;
	}

	const std::string& get_label() const
	{
		return m_label;
	}

	void set_label(std::string label)
	{
		m_label = std::move(label);
	}

	bool is_active() const
	{
		return m_active;
	}

	void set_active(bool active)
	{
		m_active = active;
	}

	friend void to_json(nlohmann::json& j, const TestConfig& c)
	{
		j = { { "x", c.get_x() }, { "label", c.get_label() }, { "active", c.is_active() } };
	}

	friend void from_json(const nlohmann::json& j, TestConfig& c)
	{
		c = TestConfig{ j.at("x").get<int>(), j.at("label").get<std::string>(), j.at("active").get<bool>() };
	}

private:
	int m_x{ 0 };
	std::string m_label;
	bool m_active{ false };
};

class ConfigurationService : public testing::Test
{
protected:
	void SetUp() override
	{
		std::filesystem::path db_path{ desktop::filesystem::user_directories::get_config() / "Test" / "app.db" };
		std::error_code ec;
		std::filesystem::remove(db_path, ec);
		std::filesystem::remove(db_path.parent_path(), ec);
		std::shared_ptr<app_info> info{ std::make_shared<app_info>("libdesktop.test.config", "Test", "Test", false) };
		std::shared_ptr<secret_service> secrets{ std::make_shared<secret_service>() };
		m_db = std::make_shared<database_service>(info, secrets);
		m_service = std::make_shared<configuration_service>(m_db);
	}

	void TearDown() override
	{
		m_service.reset();
		m_db.reset();
		std::filesystem::path db_path{ desktop::filesystem::user_directories::get_config() / "Test" / "app.db" };
		std::error_code ec;
		std::filesystem::remove(db_path, ec);
		std::filesystem::remove(db_path.parent_path(), ec);
	}

	std::shared_ptr<database_service> m_db;
	std::shared_ptr<configuration_service> m_service;
};

TEST_F(ConfigurationService, GetBoolDefaultWhenMissing)
{
	ASSERT_FALSE(m_service->get<bool>("missing_bool", false));
}

TEST_F(ConfigurationService, GetBoolDefaultTrueWhenMissing)
{
	ASSERT_TRUE(m_service->get<bool>("missing_bool", true));
}

TEST_F(ConfigurationService, SetAndGetBoolTrue)
{
	m_service->set("bool_key", true);
	ASSERT_TRUE(m_service->get<bool>("bool_key", false));
}

TEST_F(ConfigurationService, SetAndGetBoolFalse)
{
	m_service->set("bool_key", false);
	ASSERT_FALSE(m_service->get<bool>("bool_key", true));
}

TEST_F(ConfigurationService, GetIntDefaultWhenMissing)
{
	ASSERT_EQ(m_service->get<int>("missing_int", 42), 42);
}

TEST_F(ConfigurationService, SetAndGetInt)
{
	m_service->set("int_key", 123);
	ASSERT_EQ(m_service->get<int>("int_key", 0), 123);
}

TEST_F(ConfigurationService, SetAndGetIntNegative)
{
	m_service->set("int_key", -99);
	ASSERT_EQ(m_service->get<int>("int_key", 0), -99);
}

TEST_F(ConfigurationService, GetInt64DefaultWhenMissing)
{
	ASSERT_EQ(m_service->get<int64_t>("missing_int64", 9999999999LL), 9999999999LL);
}

TEST_F(ConfigurationService, SetAndGetInt64)
{
	m_service->set<int64_t>("int64_key", 9000000000LL);
	ASSERT_EQ(m_service->get<int64_t>("int64_key", 0LL), 9000000000LL);
}

TEST_F(ConfigurationService, GetFloatDefaultWhenMissing)
{
	ASSERT_FLOAT_EQ(m_service->get<float>("missing_float", 1.5f), 1.5f);
}

TEST_F(ConfigurationService, SetAndGetFloat)
{
	m_service->set("float_key", 3.14f);
	ASSERT_NEAR(m_service->get<float>("float_key", 0.0f), 3.14f, 0.001f);
}

TEST_F(ConfigurationService, GetDoubleDefaultWhenMissing)
{
	ASSERT_DOUBLE_EQ(m_service->get<double>("missing_double", 2.71828), 2.71828);
}

TEST_F(ConfigurationService, SetAndGetDouble)
{
	m_service->set("double_key", 1.23456789);
	ASSERT_NEAR(m_service->get<double>("double_key", 0.0), 1.23456789, 0.000001);
}

TEST_F(ConfigurationService, GetStringDefaultWhenMissing)
{
	ASSERT_EQ(m_service->get<std::string>("missing_str", "default"), "default");
}

TEST_F(ConfigurationService, SetAndGetString)
{
	m_service->set("str_key", std::string{ "hello world" });
	ASSERT_EQ(m_service->get<std::string>("str_key", ""), "hello world");
}

TEST_F(ConfigurationService, SetStringFromConstChar)
{
	m_service->set<const char*>("str_key", "from_const_char");
	ASSERT_EQ(m_service->get<std::string>("str_key", ""), "from_const_char");
}

TEST_F(ConfigurationService, SetStringFromStringView)
{
	std::string_view sv{ "string_view_value" };
	m_service->set("str_key", sv);
	ASSERT_EQ(m_service->get<std::string>("str_key", ""), "string_view_value");
}

TEST_F(ConfigurationService, SetAndGetEmptyString)
{
	m_service->set("empty_str", std::string{ "" });
	ASSERT_EQ(m_service->get<std::string>("empty_str", "default"), "");
}

TEST_F(ConfigurationService, OverwriteExistingValue)
{
	m_service->set("key", 1);
	m_service->set("key", 2);
	ASSERT_EQ(m_service->get<int>("key", 0), 2);
}

TEST_F(ConfigurationService, OverwriteStringValue)
{
	m_service->set("str_key", std::string{ "first" });
	m_service->set("str_key", std::string{ "second" });
	ASSERT_EQ(m_service->get<std::string>("str_key", ""), "second");
}

TEST_F(ConfigurationService, DifferentKeysAreIndependent)
{
	m_service->set("key_a", 10);
	m_service->set("key_b", 20);
	ASSERT_EQ(m_service->get<int>("key_a", 0), 10);
	ASSERT_EQ(m_service->get<int>("key_b", 0), 20);
}

TEST_F(ConfigurationService, GetAllRawEmptyWhenNoValues)
{
	ASSERT_TRUE(m_service->get_all_raw().empty());
}

TEST_F(ConfigurationService, GetAllRawContainsSetValues)
{
	m_service->set("key1", std::string{ "val1" });
	m_service->set("key2", std::string{ "val2" });
	const auto all = m_service->get_all_raw();
	ASSERT_EQ(all.size(), 2);
	ASSERT_EQ(all.at("key1"), "val1");
	ASSERT_EQ(all.at("key2"), "val2");
}

TEST_F(ConfigurationService, GetAllRawReflectsOverwrite)
{
	m_service->set("key", std::string{ "old" });
	m_service->set("key", std::string{ "new" });
	const auto all = m_service->get_all_raw();
	ASSERT_EQ(all.at("key"), "new");
}

TEST_F(ConfigurationService, GetAllRawSizeDoesNotGrowOnOverwrite)
{
	m_service->set("key", std::string{ "first" });
	m_service->set("key", std::string{ "second" });
	ASSERT_EQ(m_service->get_all_raw().size(), 1);
}

TEST_F(ConfigurationService, SavedEventFiresOnSet)
{
	bool fired{ false };
	m_service->get_saved_event().add_handler([&](const configuration_service&, const configuration_saved_event_args&)
	{
		fired = true;
	});
	m_service->set("key", 1);
	ASSERT_TRUE(fired);
}

TEST_F(ConfigurationService, SavedEventFiresWithCorrectName)
{
	std::string received_name;
	m_service->get_saved_event().add_handler([&](const configuration_service&, const configuration_saved_event_args& args)
	{
		received_name = args.get_changed_property_name();
	});
	m_service->set("my_key", 42);
	ASSERT_EQ(received_name, "my_key");
}

TEST_F(ConfigurationService, SavedEventFiresOnStringSet)
{
	bool fired{ false };
	m_service->get_saved_event().add_handler([&](const configuration_service&, const configuration_saved_event_args&)
	{
		fired = true;
	});
	m_service->set("str_key", std::string{ "value" });
	ASSERT_TRUE(fired);
}

TEST_F(ConfigurationService, SavedEventFiresOnOverwrite)
{
	int count{ 0 };
	m_service->get_saved_event().add_handler([&](const configuration_service&, const configuration_saved_event_args&)
	{
		++count;
	});
	m_service->set("key", 1);
	m_service->set("key", 2);
	ASSERT_EQ(count, 2);
}

TEST_F(ConfigurationService, SavedEventSenderMatchesService)
{
	const configuration_service* sender{ nullptr };
	m_service->get_saved_event().add_handler([&](const configuration_service& s, const configuration_saved_event_args&)
	{
		sender = &s;
	});
	m_service->set("key", 1);
	ASSERT_EQ(sender, m_service.get());
}

TEST_F(ConfigurationService, GetBoolFrom1String)
{
	m_service->set("bool_key", std::string{ "1" });
	ASSERT_TRUE(m_service->get<bool>("bool_key", false));
}

TEST_F(ConfigurationService, GetBoolFromTrueString)
{
	m_service->set("bool_key", std::string{ "true" });
	ASSERT_TRUE(m_service->get<bool>("bool_key", false));
}

TEST_F(ConfigurationService, GetBoolFromFalseString)
{
	m_service->set("bool_key", std::string{ "false" });
	ASSERT_FALSE(m_service->get<bool>("bool_key", true));
}

TEST_F(ConfigurationService, CacheReturnsSameValueOnSecondGet)
{
	m_service->set("int_key", 55);
	const int first = m_service->get<int>("int_key", 0);
	const int second = m_service->get<int>("int_key", 0);
	ASSERT_EQ(first, second);
}

TEST_F(ConfigurationService, SetAndGetEnum)
{
	m_service->set("enum_key", TestEnum::beta);
	ASSERT_EQ(m_service->get<TestEnum>("enum_key", TestEnum::alpha), TestEnum::beta);
}

TEST_F(ConfigurationService, GetEnumDefaultWhenMissing)
{
	ASSERT_EQ(m_service->get<TestEnum>("missing_enum", TestEnum::gamma), TestEnum::gamma);
}

TEST_F(ConfigurationService, GetObjectDefaultWhenMissing)
{
	TestConfig def{ 7, "default_label", false };
	TestConfig result = m_service->get<TestConfig>("missing_obj", def);
	ASSERT_EQ(result.get_x(), 7);
	ASSERT_EQ(result.get_label(), "default_label");
	ASSERT_FALSE(result.is_active());
}

TEST_F(ConfigurationService, SetAndGetObject)
{
	TestConfig cfg{ 42, "hello", true };
	m_service->set("obj_key", cfg);
	TestConfig result = m_service->get<TestConfig>("obj_key", {});
	ASSERT_EQ(result.get_x(), 42);
	ASSERT_EQ(result.get_label(), "hello");
	ASSERT_TRUE(result.is_active());
}

TEST_F(ConfigurationService, OverwriteObject)
{
	TestConfig first{ 1, "first", false };
	TestConfig second{ 2, "second", true };
	m_service->set("obj_key", first);
	m_service->set("obj_key", second);
	TestConfig result = m_service->get<TestConfig>("obj_key", {});
	ASSERT_EQ(result.get_x(), 2);
	ASSERT_EQ(result.get_label(), "second");
	ASSERT_TRUE(result.is_active());
}

TEST_F(ConfigurationService, ObjectOverwriteDoesNotGrowAllRaw)
{
	m_service->set("obj_key", TestConfig{ 1, "a", false });
	m_service->set("obj_key", TestConfig{ 2, "b", true });
	ASSERT_EQ(m_service->get_all_raw().size(), 1);
}

TEST_F(ConfigurationService, SetObjectFiresSavedEvent)
{
	bool fired{ false };
	m_service->get_saved_event().add_handler([&](const configuration_service&, const configuration_saved_event_args&)
	{
		fired = true;
	});
	m_service->set("obj_key", TestConfig{ 1, "x", true });
	ASSERT_TRUE(fired);
}

TEST_F(ConfigurationService, SetObjectSavedEventHasCorrectName)
{
	std::string received_name;
	m_service->get_saved_event().add_handler([&](const configuration_service&, const configuration_saved_event_args& args)
	{
		received_name = args.get_changed_property_name();
	});
	m_service->set("my_obj", TestConfig{ 0, "", false });
	ASSERT_EQ(received_name, "my_obj");
}

TEST_F(ConfigurationService, GetAllRawContainsObjectAsJson)
{
	m_service->set("obj_key", TestConfig{ 5, "test", true });
	const auto all = m_service->get_all_raw();
	ASSERT_TRUE(all.contains("obj_key"));
	ASSERT_FALSE(all.at("obj_key").empty());
}

TEST_F(ConfigurationService, MixedPrimitiveAndObjectKeysCoexist)
{
	m_service->set("int_key", 7);
	m_service->set("obj_key", TestConfig{ 3, "mixed", false });
	ASSERT_EQ(m_service->get<int>("int_key", 0), 7);
	TestConfig result = m_service->get<TestConfig>("obj_key", {});
	ASSERT_EQ(result.get_x(), 3);
}