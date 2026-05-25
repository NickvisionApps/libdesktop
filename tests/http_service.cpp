#include <fstream>
#include <gtest/gtest.h>
#include <libdesktop.h>

using namespace desktop::filesystem;
using namespace desktop::network;
using namespace desktop::system;

class httpbin_data
{
public:
	httpbin_data() = default;

	httpbin_data(std::string origin, std::string url)
	    : m_origin{ std::move(origin) },
	      m_url{ std::move(url) }
	{
	}

	const std::string& get_origin() const
	{
		return m_origin;
	}

	const std::string& get_url() const
	{
		return m_url;
	}

	friend void to_json(nlohmann::json& j, const httpbin_data& d)
	{
		j = { { "origin", d.get_origin() }, { "url", d.get_url() } };
	}

	friend void from_json(const nlohmann::json& j, httpbin_data& d)
	{
		d = httpbin_data{ j.at("origin").get<std::string>(), j.at("url").get<std::string>() };
	}

private:
	std::string m_origin;
	std::string m_url;
};

TEST(HttpService, Del)
{
	http_service service;
	http_response response;
	ASSERT_NO_THROW(response = service.del("https://httpbin.org/delete"));
	ASSERT_TRUE(response.is_success());
	ASSERT_EQ(response.get_response_code(), 200);
	std::optional<httpbin_data> data{ std::nullopt };
	ASSERT_FALSE(response.get_content_as_string().empty());
	ASSERT_NO_THROW(data = response.get_content_as_object<httpbin_data>());
	ASSERT_TRUE(data.has_value());
	ASSERT_EQ(data->get_url(), "https://httpbin.org/delete");
	ASSERT_FALSE(data->get_origin().empty());
}

TEST(HttpService, DownloadFile)
{
	http_service service;
	std::filesystem::path destination{ user_directories::get_cache() / "test.png" };
	if (std::filesystem::exists(destination))
	{
		std::filesystem::remove(destination);
	}
	ASSERT_FALSE(std::filesystem::exists(destination));
	ASSERT_TRUE(service.download_file("https://httpbin.org/image/png", destination));
	ASSERT_TRUE(std::filesystem::exists(destination));
	ASSERT_TRUE(std::filesystem::file_size(destination) > 0);
	ASSERT_TRUE(std::filesystem::is_regular_file(destination));
	std::filesystem::remove(destination);
}

TEST(HttpService, DownloadFileWithProgress)
{
	http_service service;
	std::filesystem::path destination{ user_directories::get_cache() / "test.png" };
	if (std::filesystem::exists(destination))
	{
		std::filesystem::remove(destination);
	}
	std::int64_t last_received = 0;
	std::int64_t last_total = 0;
	ASSERT_FALSE(std::filesystem::exists(destination));
	ASSERT_TRUE(service.download_file("https://httpbin.org/image/png", destination, true, [&last_received, &last_total](const download_progress& progress)
	{
		last_received = progress.get_received();
		last_total = progress.get_total();
	}));
	ASSERT_TRUE(std::filesystem::exists(destination));
	ASSERT_TRUE(std::filesystem::file_size(destination) > 0);
	ASSERT_TRUE(std::filesystem::is_regular_file(destination));
	ASSERT_TRUE(last_total > 0);
	ASSERT_EQ(last_received, last_total);
	std::filesystem::remove(destination);
}

TEST(HttpService, Get)
{
	http_service service;
	http_response response;
	ASSERT_NO_THROW(response = service.get("https://httpbin.org/get"));
	ASSERT_TRUE(response.is_success());
	ASSERT_EQ(response.get_response_code(), 200);
	std::optional<httpbin_data> data{ std::nullopt };
	ASSERT_FALSE(response.get_content_as_string().empty());
	ASSERT_NO_THROW(data = response.get_content_as_object<httpbin_data>());
	ASSERT_TRUE(data.has_value());
	ASSERT_EQ(data->get_url(), "https://httpbin.org/get");
	ASSERT_FALSE(data->get_origin().empty());
}

TEST(HttpService, PatchJson)
{
	http_service service;
	http_response response;
	nlohmann::json body{ { "key", "value" } };
	ASSERT_NO_THROW(response = service.patch("https://httpbin.org/patch", body));
	ASSERT_TRUE(response.is_success());
	ASSERT_EQ(response.get_response_code(), 200);
	std::optional<httpbin_data> data{ std::nullopt };
	ASSERT_FALSE(response.get_content_as_string().empty());
	ASSERT_NO_THROW(data = response.get_content_as_object<httpbin_data>());
	ASSERT_TRUE(data.has_value());
	ASSERT_EQ(data->get_url(), "https://httpbin.org/patch");
	ASSERT_FALSE(data->get_origin().empty());
}

TEST(HttpService, PatchData)
{
	http_service service;
	http_response response;
	std::vector<std::byte> bytes{ std::byte{ 0x01 }, std::byte{ 0x02 }, std::byte{ 0x03 } };
	ASSERT_NO_THROW(response = service.patch("https://httpbin.org/patch", bytes));
	ASSERT_TRUE(response.is_success());
	ASSERT_EQ(response.get_response_code(), 200);
	std::optional<httpbin_data> data{ std::nullopt };
	ASSERT_FALSE(response.get_content_as_string().empty());
	ASSERT_NO_THROW(data = response.get_content_as_object<httpbin_data>());
	ASSERT_TRUE(data.has_value());
	ASSERT_EQ(data->get_url(), "https://httpbin.org/patch");
	ASSERT_FALSE(data->get_origin().empty());
}

TEST(HttpService, PatchFile)
{
	http_service service;
	std::filesystem::path file{ user_directories::get_cache() / "test_patch.txt" };
	std::ofstream stream{ file };
	stream << "patch file content";
	stream.close();
	http_response response;
	ASSERT_NO_THROW(response = service.patch("https://httpbin.org/patch", file));
	ASSERT_TRUE(response.is_success());
	ASSERT_EQ(response.get_response_code(), 200);
	std::filesystem::remove(file);
	std::optional<httpbin_data> data{ std::nullopt };
	ASSERT_FALSE(response.get_content_as_string().empty());
	ASSERT_NO_THROW(data = response.get_content_as_object<httpbin_data>());
	ASSERT_TRUE(data.has_value());
	ASSERT_EQ(data->get_url(), "https://httpbin.org/patch");
	ASSERT_FALSE(data->get_origin().empty());
}

TEST(HttpService, PatchForm)
{
	http_service service;
	http_response response;
	std::vector<std::pair<std::string, std::string>> form{ { "field1", "value1" }, { "field2", "value2" } };
	ASSERT_NO_THROW(response = service.patch("https://httpbin.org/patch", form));
	ASSERT_TRUE(response.is_success());
	ASSERT_EQ(response.get_response_code(), 200);
	std::optional<httpbin_data> data{ std::nullopt };
	ASSERT_FALSE(response.get_content_as_string().empty());
	ASSERT_NO_THROW(data = response.get_content_as_object<httpbin_data>());
	ASSERT_TRUE(data.has_value());
	ASSERT_EQ(data->get_url(), "https://httpbin.org/patch");
	ASSERT_FALSE(data->get_origin().empty());
}

TEST(HttpService, Ping)
{
	http_service service;
	ASSERT_TRUE(service.ping("https://httpbin.org/get"));
	ASSERT_FALSE(service.ping("https://invalid.url"));
}

TEST(HttpService, PostJson)
{
	http_service service;
	http_response response;
	nlohmann::json body{ { "key", "value" } };
	ASSERT_NO_THROW(response = service.post("https://httpbin.org/post", body));
	ASSERT_TRUE(response.is_success());
	ASSERT_EQ(response.get_response_code(), 200);
	std::optional<httpbin_data> data{ std::nullopt };
	ASSERT_FALSE(response.get_content_as_string().empty());
	ASSERT_NO_THROW(data = response.get_content_as_object<httpbin_data>());
	ASSERT_TRUE(data.has_value());
	ASSERT_EQ(data->get_url(), "https://httpbin.org/post");
	ASSERT_FALSE(data->get_origin().empty());
}

TEST(HttpService, PostData)
{
	http_service service;
	http_response response;
	std::vector<std::byte> bytes{ std::byte{ 0x01 }, std::byte{ 0x02 }, std::byte{ 0x03 } };
	ASSERT_NO_THROW(response = service.post("https://httpbin.org/post", bytes));
	ASSERT_TRUE(response.is_success());
	ASSERT_EQ(response.get_response_code(), 200);
	std::optional<httpbin_data> data{ std::nullopt };
	ASSERT_FALSE(response.get_content_as_string().empty());
	ASSERT_NO_THROW(data = response.get_content_as_object<httpbin_data>());
	ASSERT_TRUE(data.has_value());
	ASSERT_EQ(data->get_url(), "https://httpbin.org/post");
	ASSERT_FALSE(data->get_origin().empty());
}

TEST(HttpService, PostFile)
{
	http_service service;
	std::filesystem::path file{ user_directories::get_cache() / "test_post.txt" };
	std::ofstream stream{ file };
	stream << "post file content";
	stream.close();
	http_response response;
	ASSERT_NO_THROW(response = service.post("https://httpbin.org/post", file));
	ASSERT_TRUE(response.is_success());
	ASSERT_EQ(response.get_response_code(), 200);
	std::filesystem::remove(file);
	std::optional<httpbin_data> data{ std::nullopt };
	ASSERT_FALSE(response.get_content_as_string().empty());
	ASSERT_NO_THROW(data = response.get_content_as_object<httpbin_data>());
	ASSERT_TRUE(data.has_value());
	ASSERT_EQ(data->get_url(), "https://httpbin.org/post");
	ASSERT_FALSE(data->get_origin().empty());
}

TEST(HttpService, PostForm)
{
	http_service service;
	http_response response;
	std::vector<std::pair<std::string, std::string>> form{ { "field1", "value1" }, { "field2", "value2" } };
	ASSERT_NO_THROW(response = service.post("https://httpbin.org/post", form));
	ASSERT_TRUE(response.is_success());
	ASSERT_EQ(response.get_response_code(), 200);
	std::optional<httpbin_data> data{ std::nullopt };
	ASSERT_FALSE(response.get_content_as_string().empty());
	ASSERT_NO_THROW(data = response.get_content_as_object<httpbin_data>());
	ASSERT_TRUE(data.has_value());
	ASSERT_EQ(data->get_url(), "https://httpbin.org/post");
	ASSERT_FALSE(data->get_origin().empty());
}

TEST(HttpService, PutJson)
{
	http_service service;
	http_response response;
	nlohmann::json body{ { "key", "value" } };
	ASSERT_NO_THROW(response = service.put("https://httpbin.org/put", body));
	ASSERT_TRUE(response.is_success());
	ASSERT_EQ(response.get_response_code(), 200);
	std::optional<httpbin_data> data{ std::nullopt };
	ASSERT_FALSE(response.get_content_as_string().empty());
	ASSERT_NO_THROW(data = response.get_content_as_object<httpbin_data>());
	ASSERT_TRUE(data.has_value());
	ASSERT_EQ(data->get_url(), "https://httpbin.org/put");
	ASSERT_FALSE(data->get_origin().empty());
}

TEST(HttpService, PutData)
{
	http_service service;
	http_response response;
	std::vector<std::byte> bytes{ std::byte{ 0x01 }, std::byte{ 0x02 }, std::byte{ 0x03 } };
	ASSERT_NO_THROW(response = service.put("https://httpbin.org/put", bytes));
	ASSERT_TRUE(response.is_success());
	ASSERT_EQ(response.get_response_code(), 200);
	std::optional<httpbin_data> data{ std::nullopt };
	ASSERT_FALSE(response.get_content_as_string().empty());
	ASSERT_NO_THROW(data = response.get_content_as_object<httpbin_data>());
	ASSERT_TRUE(data.has_value());
	ASSERT_EQ(data->get_url(), "https://httpbin.org/put");
	ASSERT_FALSE(data->get_origin().empty());
}

TEST(HttpService, PutFile)
{
	http_service service;
	std::filesystem::path file{ user_directories::get_cache() / "test_put.txt" };
	std::ofstream stream{ file };
	stream << "put file content";
	stream.close();
	http_response response;
	ASSERT_NO_THROW(response = service.put("https://httpbin.org/put", file));
	ASSERT_TRUE(response.is_success());
	ASSERT_EQ(response.get_response_code(), 200);
	std::filesystem::remove(file);
	std::optional<httpbin_data> data{ std::nullopt };
	ASSERT_FALSE(response.get_content_as_string().empty());
	ASSERT_NO_THROW(data = response.get_content_as_object<httpbin_data>());
	ASSERT_TRUE(data.has_value());
	ASSERT_EQ(data->get_url(), "https://httpbin.org/put");
	ASSERT_FALSE(data->get_origin().empty());
}

TEST(HttpService, PutForm)
{
	http_service service;
	http_response response;
	std::vector<std::pair<std::string, std::string>> form{ { "field1", "value1" }, { "field2", "value2" } };
	ASSERT_NO_THROW(response = service.put("https://httpbin.org/put", form));
	ASSERT_TRUE(response.is_success());
	ASSERT_EQ(response.get_response_code(), 200);
	std::optional<httpbin_data> data{ std::nullopt };
	ASSERT_FALSE(response.get_content_as_string().empty());
	ASSERT_NO_THROW(data = response.get_content_as_object<httpbin_data>());
	ASSERT_TRUE(data.has_value());
	ASSERT_EQ(data->get_url(), "https://httpbin.org/put");
	ASSERT_FALSE(data->get_origin().empty());
}

TEST(HttpService, SSL)
{
	http_service service;
	ASSERT_TRUE(service.is_ssl_verification_enabled());
	service.disable_ssl_verification();
	ASSERT_FALSE(service.is_ssl_verification_enabled());
	service.enable_ssl_verification();
	ASSERT_TRUE(service.is_ssl_verification_enabled());
}

TEST(HttpService, ValidUrl)
{
	http_service service;
	ASSERT_TRUE(service.is_valid_url("http://example.com"));
	ASSERT_TRUE(service.is_valid_url("https://example.com"));
	ASSERT_TRUE(service.is_valid_url("ftp://example.com"));
	ASSERT_TRUE(service.is_valid_url("https://example.com/path/to/resource"));
	ASSERT_TRUE(service.is_valid_url("https://example.com/path?query=value"));
	ASSERT_TRUE(service.is_valid_url("https://example.com/path?a=1&b=2"));
	ASSERT_TRUE(service.is_valid_url("https://example.com/path#fragment"));
	ASSERT_TRUE(service.is_valid_url("https://example.com:8080"));
	ASSERT_TRUE(service.is_valid_url("https://example.com:8080/path"));
	ASSERT_TRUE(service.is_valid_url("https://user:password@example.com"));
	ASSERT_TRUE(service.is_valid_url("https://subdomain.example.com"));
	ASSERT_TRUE(service.is_valid_url("https://sub.sub.example.com"));
	ASSERT_TRUE(service.is_valid_url("https://example.com/path/to/resource?query=value&a=1#fragment"));
	ASSERT_TRUE(service.is_valid_url("https://192.168.1.1"));
	ASSERT_TRUE(service.is_valid_url("https://192.168.1.1:8080/path"));
	ASSERT_FALSE(service.is_valid_url(""));
	ASSERT_FALSE(service.is_valid_url("example.com"));
	ASSERT_FALSE(service.is_valid_url("//example.com"));
	ASSERT_FALSE(service.is_valid_url("://example.com"));
	ASSERT_FALSE(service.is_valid_url("https://"));
	ASSERT_FALSE(service.is_valid_url("https:// example.com"));
	ASSERT_FALSE(service.is_valid_url("not a url"));
	ASSERT_FALSE(service.is_valid_url("http://"));
	ASSERT_FALSE(service.is_valid_url("http:// "));
}