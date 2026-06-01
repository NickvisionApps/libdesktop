#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <libdesktop.h>
#include <ranges>

using namespace desktop::app;
using namespace desktop::system;

class TranslationService : public testing::Test
{
protected:
	void SetUp() override
	{
		std::filesystem::path executable_dir{ environment::get_executable_directory() };
		std::error_code ec;
		std::filesystem::remove_all(executable_dir / "zz_test_lang_a", ec);
		std::filesystem::remove_all(executable_dir / "zz_test_lang_b", ec);
		std::filesystem::path mo_path_a{ executable_dir / "zz_test_lang_a" / "LC_MESSAGES" / "translationservicetests.mo" };
		std::filesystem::path mo_path_b{ executable_dir / "zz_test_lang_b" / "LC_MESSAGES" / "translationservicetests.mo" };
		std::filesystem::create_directories(mo_path_a.parent_path(), ec);
		std::filesystem::create_directories(mo_path_b.parent_path(), ec);
		std::ofstream out_a{ mo_path_a };
		out_a << "not-a-real-mo";
		std::ofstream out_b{ mo_path_b };
		out_b << "not-a-real-mo";
		m_info = std::make_shared<app_info>("libdesktop.test.translation", "TranslationServiceTests", "TranslationServiceTests", false);
		m_service = std::make_shared<translation_service>(m_info);
	}

	void TearDown() override
	{
		m_service.reset();
		m_info.reset();
		environment::clear_variable("LANGUAGE");
		std::filesystem::path executable_dir{ environment::get_executable_directory() };
		std::error_code ec;
		std::filesystem::remove_all(executable_dir / "zz_test_lang_a", ec);
		std::filesystem::remove_all(executable_dir / "zz_test_lang_b", ec);
	}

	std::shared_ptr<app_info> m_info;
	std::shared_ptr<translation_service> m_service;
};

TEST_F(TranslationService, InitialLanguageIsC)
{
	ASSERT_EQ(m_service->get_language(), "C");
}

TEST_F(TranslationService, SetLanguageCDisablesTranslations)
{
	ASSERT_TRUE(m_service->set_language("C"));
	ASSERT_EQ(m_service->get_language(), "C");
	ASSERT_STREQ(m_service->_("hello"), "hello");
}

TEST_F(TranslationService, SetLanguageEmptyClearsLanguageVariable)
{
	environment::set_variable("LANGUAGE", "zz_test_lang_a");
	ASSERT_TRUE(m_service->set_language(""));
	ASSERT_EQ(m_service->get_language(), "");
	ASSERT_FALSE(environment::has_variable("LANGUAGE"));
}

TEST_F(TranslationService, SetUnavailableLanguageReturnsFalse)
{
	ASSERT_FALSE(m_service->set_language("language_that_does_not_exist"));
	ASSERT_EQ(m_service->get_language(), "C");
}

TEST_F(TranslationService, GetAvailableLanguagesIncludesExecutableLocales)
{
	std::vector<std::string> languages{ m_service->get_available_languages() };
	ASSERT_TRUE(std::ranges::find(languages, "zz_test_lang_a") != languages.end());
	ASSERT_TRUE(std::ranges::find(languages, "zz_test_lang_b") != languages.end());
}

TEST_F(TranslationService, SetAvailableLanguageReturnsTrue)
{
	ASSERT_TRUE(m_service->set_language("zz_test_lang_a"));
	ASSERT_EQ(m_service->get_language(), "zz_test_lang_a");
	ASSERT_EQ(environment::get_variable("LANGUAGE"), "zz_test_lang_a");
}

TEST_F(TranslationService, PluralFallbackWhenTranslationsDisabled)
{
	ASSERT_TRUE(m_service->set_language("C"));
	ASSERT_STREQ(m_service->_n("item", "items", 1), "item");
	ASSERT_STREQ(m_service->_n("item", "items", 2), "items");
}

TEST_F(TranslationService, ContextFallbackWhenTranslationsDisabled)
{
	ASSERT_TRUE(m_service->set_language("C"));
	ASSERT_STREQ(m_service->_p("ctx", "value"), "value");
	ASSERT_STREQ(m_service->_pn("ctx", "entry", "entries", 1), "entry");
	ASSERT_STREQ(m_service->_pn("ctx", "entry", "entries", 2), "entries");
}

TEST_F(TranslationService, FormattedTranslationHelpersWorkInCMode)
{
	ASSERT_TRUE(m_service->set_language("C"));
	ASSERT_EQ(m_service->_("Count: {}", 3), "Count: 3");
	ASSERT_EQ(m_service->_n("{} file", "{} files", 2, 2), "2 files");
}
