#include "network/http_service.h"
#include <curl/curl.h>
#include <fstream>
#include <stdexcept>

static const std::string& get_user_agent()
{
#ifdef _WIN32
	static std::string user_agent{
		"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/147.0.0.0 Safari/537.36 Edg/148.0.3967.54"
	};
#elif defined(__linux__)
	static std::string user_agent{ "Mozilla/5.0 (X11; Linux x86_64; rv:150.0) Gecko/20100101 Firefox/150.0" };
#elif defined(__APPLE__)
	static std::string user_agent{ "Mozilla/5.0 (Macintosh; Intel Mac OS X 15_7_5) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/26.0 Safari/605.1.15" };
#endif
	return user_agent;
}

namespace desktop::network
{
	http_service::http_service()
	{
		CURLcode result{ curl_global_init(CURL_GLOBAL_ALL) };
		if (result != CURLE_OK)
		{
			throw std::runtime_error("Unable to initalize curl");
		}
	}

	http_service::~http_service()
	{
		curl_global_cleanup();
	}

	http_response http_service::del(const std::string& url) const
	{
		if (url.empty())
		{
			return {};
		}
		CURL* easy{ curl_easy_init() };
		if (easy == nullptr)
		{
			return {};
		}
		http_response response;
		curl_easy_setopt(easy, CURLOPT_URL, url.c_str());
		if (!m_ssl_verification)
		{
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYHOST, 0L);
		}
		curl_easy_setopt(easy, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(easy, CURLOPT_USERAGENT, get_user_agent().c_str());
		curl_easy_setopt(easy, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(easy, CURLOPT_WRITEDATA, &response);
		curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, +[](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t
		{
			http_response* response{ static_cast<http_response*>(userdata) };
			response->write_content(ptr, size * nmemb);
			return size * nmemb;
		});
		curl_easy_setopt(easy, CURLOPT_CUSTOMREQUEST, "DELETE");
		if (curl_easy_perform(easy) == CURLE_OK)
		{
			response.set_info_from_easy(easy);
		}
		curl_easy_cleanup(easy);
		return response;
	}

	void http_service::disable_ssl_verification()
	{
		m_ssl_verification = false;
	}

	bool http_service::download_file(const std::string& url, const std::filesystem::path& destination, bool overwrite,
	                                 const std::function<void(const download_progress&)>& progress) const
	{
		if (url.empty())
		{
			return false;
		}
		if (std::filesystem::exists(destination))
		{
			if (!overwrite)
			{
				return false;
			}
			std::filesystem::remove(destination);
		}
		std::ofstream out{ destination, std::ios::binary };
		CURL* easy{ curl_easy_init() };
		if (easy == nullptr)
		{
			out.close();
			std::filesystem::remove(destination);
			return false;
		}
		curl_easy_setopt(easy, CURLOPT_URL, url.c_str());
		if (!m_ssl_verification)
		{
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYHOST, 0L);
		}
		curl_easy_setopt(easy, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(easy, CURLOPT_USERAGENT, get_user_agent().c_str());
		if (progress)
		{
			curl_easy_setopt(easy, CURLOPT_NOPROGRESS, 0L);
			curl_easy_setopt(easy, CURLOPT_XFERINFODATA, &progress);
			curl_easy_setopt(easy, CURLOPT_XFERINFOFUNCTION, +[](void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t, curl_off_t) -> int
			{
				std::function<void(const download_progress&)>* callback{ static_cast<std::function<void(const download_progress&)>*>(clientp) };
				(*callback)({ static_cast<std::int64_t>(dlnow), static_cast<std::int64_t>(dltotal) });
				return 0;
			});
		}
		else
		{
			curl_easy_setopt(easy, CURLOPT_NOPROGRESS, 1L);
		}
		curl_easy_setopt(easy, CURLOPT_WRITEDATA, &out);
		curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, +[](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t
		{
			std::ofstream* out{ static_cast<std::ofstream*>(userdata) };
			out->write(ptr, static_cast<std::streamsize>(size * nmemb));
			return size * nmemb;
		});
		if (curl_easy_perform(easy) == CURLE_OK)
		{
			http_response response;
			response.set_info_from_easy(easy);
			if (response.is_success())
			{
				curl_easy_cleanup(easy);
				out.close();
				return true;
			}
		}
		curl_easy_cleanup(easy);
		out.close();
		std::filesystem::remove(destination);
		return false;
	}

	void http_service::enable_ssl_verification()
	{
		m_ssl_verification = true;
	}

	http_response http_service::get(const std::string& url) const
	{
		if (url.empty())
		{
			return {};
		}
		CURL* easy{ curl_easy_init() };
		if (easy == nullptr)
		{
			return {};
		}
		http_response response;
		curl_easy_setopt(easy, CURLOPT_URL, url.c_str());
		if (!m_ssl_verification)
		{
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYHOST, 0L);
		}
		curl_easy_setopt(easy, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(easy, CURLOPT_USERAGENT, get_user_agent().c_str());
		curl_easy_setopt(easy, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(easy, CURLOPT_WRITEDATA, &response);
		curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, +[](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t
		{
			http_response* response{ static_cast<http_response*>(userdata) };
			response->write_content(ptr, size * nmemb);
			return size * nmemb;
		});
		if (curl_easy_perform(easy) == CURLE_OK)
		{
			response.set_info_from_easy(easy);
		}
		curl_easy_cleanup(easy);
		return response;
	}

	bool http_service::is_ssl_verification_enabled() const
	{
		return m_ssl_verification;
	}

	bool http_service::is_valid_url(const std::string& url) const
	{
		if (url.empty())
		{
			return false;
		}
		CURLU* curl{ curl_url() };
		if (curl == nullptr)
		{
			return false;
		}
		CURLUcode result{ curl_url_set(curl, CURLUPART_URL, url.c_str(), 0) };
		curl_url_cleanup(curl);
		return result == CURLUE_OK;
	}

	http_response http_service::patch(const std::string& url, const nlohmann::json& json) const
	{
		if (url.empty())
		{
			return {};
		}
		CURL* easy{ curl_easy_init() };
		if (easy == nullptr)
		{
			return {};
		}
		std::string body{ json.dump() };
		curl_slist* headers{ nullptr };
		headers = curl_slist_append(headers, "Content-Type: application/json");
		http_response response;
		curl_easy_setopt(easy, CURLOPT_URL, url.c_str());
		if (!m_ssl_verification)
		{
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYHOST, 0L);
		}
		curl_easy_setopt(easy, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(easy, CURLOPT_USERAGENT, get_user_agent().c_str());
		curl_easy_setopt(easy, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(easy, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(easy, CURLOPT_CUSTOMREQUEST, "PATCH");
		curl_easy_setopt(easy, CURLOPT_POSTFIELDS, body.c_str());
		curl_easy_setopt(easy, CURLOPT_POSTFIELDSIZE, static_cast<curl_off_t>(body.size()));
		curl_easy_setopt(easy, CURLOPT_WRITEDATA, &response);
		curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, +[](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t
		{
			http_response* response{ static_cast<http_response*>(userdata) };
			response->write_content(ptr, size * nmemb);
			return size * nmemb;
		});
		if (curl_easy_perform(easy) == CURLE_OK)
		{
			response.set_info_from_easy(easy);
		}
		curl_easy_cleanup(easy);
		curl_slist_free_all(headers);
		return response;
	}

	http_response http_service::patch(const std::string& url, const std::vector<std::byte>& data) const
	{
		if (url.empty())
		{
			return {};
		}
		CURL* easy{ curl_easy_init() };
		if (easy == nullptr)
		{
			return {};
		}
		curl_slist* headers{ nullptr };
		headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
		http_response response;
		curl_easy_setopt(easy, CURLOPT_URL, url.c_str());
		if (!m_ssl_verification)
		{
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYHOST, 0L);
		}
		curl_easy_setopt(easy, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(easy, CURLOPT_USERAGENT, get_user_agent().c_str());
		curl_easy_setopt(easy, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(easy, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(easy, CURLOPT_CUSTOMREQUEST, "PATCH");
		curl_easy_setopt(easy, CURLOPT_POSTFIELDS, reinterpret_cast<const char*>(data.data()));
		curl_easy_setopt(easy, CURLOPT_POSTFIELDSIZE, static_cast<curl_off_t>(data.size()));
		curl_easy_setopt(easy, CURLOPT_WRITEDATA, &response);
		curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, +[](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t
		{
			http_response* response{ static_cast<http_response*>(userdata) };
			response->write_content(ptr, size * nmemb);
			return size * nmemb;
		});
		if (curl_easy_perform(easy) == CURLE_OK)
		{
			response.set_info_from_easy(easy);
		}
		curl_easy_cleanup(easy);
		curl_slist_free_all(headers);
		return response;
	}

	http_response http_service::patch(const std::string& url, const std::filesystem::path& file_path) const
	{
		if (url.empty() || !std::filesystem::exists(file_path))
		{
			return {};
		}
		CURL* easy{ curl_easy_init() };
		if (easy == nullptr)
		{
			return {};
		}
		std::ifstream in{ file_path, std::ios::binary };
		if (!in.is_open())
		{
			curl_easy_cleanup(easy);
			return {};
		}
		curl_slist* headers{ nullptr };
		headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
		http_response response;
		curl_easy_setopt(easy, CURLOPT_URL, url.c_str());
		if (!m_ssl_verification)
		{
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYHOST, 0L);
		}
		curl_easy_setopt(easy, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(easy, CURLOPT_USERAGENT, get_user_agent().c_str());
		curl_easy_setopt(easy, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(easy, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(easy, CURLOPT_CUSTOMREQUEST, "PATCH");
		curl_easy_setopt(easy, CURLOPT_POST, 1L);
		curl_easy_setopt(easy, CURLOPT_POSTFIELDSIZE_LARGE, static_cast<curl_off_t>(std::filesystem::file_size(file_path)));
		curl_easy_setopt(easy, CURLOPT_READDATA, &in);
		curl_easy_setopt(easy, CURLOPT_READFUNCTION, +[](char* buffer, size_t size, size_t nmemb, void* userdata) -> size_t
		{
			std::ifstream* in{ static_cast<std::ifstream*>(userdata) };
			in->read(buffer, static_cast<std::streamsize>(size * nmemb));
			return static_cast<size_t>(in->gcount());
		});
		curl_easy_setopt(easy, CURLOPT_WRITEDATA, &response);
		curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, +[](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t
		{
			http_response* response{ static_cast<http_response*>(userdata) };
			response->write_content(ptr, size * nmemb);
			return size * nmemb;
		});
		if (curl_easy_perform(easy) == CURLE_OK)
		{
			response.set_info_from_easy(easy);
		}
		curl_easy_cleanup(easy);
		curl_slist_free_all(headers);
		return response;
	}

	http_response http_service::patch(const std::string& url, const std::vector<std::pair<std::string, std::string>>& form) const
	{
		if (url.empty())
		{
			return {};
		}
		CURL* easy{ curl_easy_init() };
		if (easy == nullptr)
		{
			return {};
		}
		curl_mime* mime{ curl_mime_init(easy) };
		for (const std::pair<std::string, std::string>& pair : form)
		{
			curl_mimepart* part{ curl_mime_addpart(mime) };
			curl_mime_name(part, pair.first.c_str());
			curl_mime_data(part, pair.second.c_str(), CURL_ZERO_TERMINATED);
		}
		http_response response;
		curl_easy_setopt(easy, CURLOPT_URL, url.c_str());
		if (!m_ssl_verification)
		{
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYHOST, 0L);
		}
		curl_easy_setopt(easy, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(easy, CURLOPT_USERAGENT, get_user_agent().c_str());
		curl_easy_setopt(easy, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(easy, CURLOPT_CUSTOMREQUEST, "PATCH");
		curl_easy_setopt(easy, CURLOPT_MIMEPOST, mime);
		curl_easy_setopt(easy, CURLOPT_WRITEDATA, &response);
		curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, +[](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t
		{
			http_response* response{ static_cast<http_response*>(userdata) };
			response->write_content(ptr, size * nmemb);
			return size * nmemb;
		});
		if (curl_easy_perform(easy) == CURLE_OK)
		{
			response.set_info_from_easy(easy);
		}
		curl_easy_cleanup(easy);
		curl_mime_free(mime);
		return response;
	}

	bool http_service::ping(const std::string& url) const
	{
		if (url.empty())
		{
			return false;
		}
		CURL* easy{ curl_easy_init() };
		if (easy == nullptr)
		{
			return false;
		}
		curl_easy_setopt(easy, CURLOPT_URL, url.c_str());
		if (!m_ssl_verification)
		{
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYHOST, 0L);
		}
		curl_easy_setopt(easy, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(easy, CURLOPT_USERAGENT, get_user_agent().c_str());
		curl_easy_setopt(easy, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(easy, CURLOPT_NOBODY, 1L);
		bool success{ curl_easy_perform(easy) == CURLE_OK };
		curl_easy_cleanup(easy);
		return success;
	}

	http_response http_service::post(const std::string& url, const nlohmann::json& json) const
	{
		if (url.empty())
		{
			return {};
		}
		CURL* easy{ curl_easy_init() };
		if (easy == nullptr)
		{
			return {};
		}
		const std::string body{ json.dump() };
		curl_slist* headers{ nullptr };
		headers = curl_slist_append(headers, "Content-Type: application/json");
		http_response response;
		curl_easy_setopt(easy, CURLOPT_URL, url.c_str());
		if (!m_ssl_verification)
		{
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYHOST, 0L);
		}
		curl_easy_setopt(easy, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(easy, CURLOPT_USERAGENT, get_user_agent().c_str());
		curl_easy_setopt(easy, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(easy, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(easy, CURLOPT_POSTFIELDS, body.c_str());
		curl_easy_setopt(easy, CURLOPT_POSTFIELDSIZE, static_cast<curl_off_t>(body.size()));
		curl_easy_setopt(easy, CURLOPT_WRITEDATA, &response);
		curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, +[](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t
		{
			http_response* response{ static_cast<http_response*>(userdata) };
			response->write_content(ptr, size * nmemb);
			return size * nmemb;
		});
		if (curl_easy_perform(easy) == CURLE_OK)
		{
			response.set_info_from_easy(easy);
		}
		curl_easy_cleanup(easy);
		curl_slist_free_all(headers);
		return response;
	}

	http_response http_service::post(const std::string& url, const std::vector<std::byte>& data) const
	{
		if (url.empty())
		{
			return {};
		}
		CURL* easy{ curl_easy_init() };
		if (easy == nullptr)
		{
			return {};
		}
		curl_slist* headers{ nullptr };
		headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
		http_response response;
		curl_easy_setopt(easy, CURLOPT_URL, url.c_str());
		if (!m_ssl_verification)
		{
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYHOST, 0L);
		}
		curl_easy_setopt(easy, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(easy, CURLOPT_USERAGENT, get_user_agent().c_str());
		curl_easy_setopt(easy, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(easy, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(easy, CURLOPT_POSTFIELDS, reinterpret_cast<const char*>(data.data()));
		curl_easy_setopt(easy, CURLOPT_POSTFIELDSIZE, static_cast<curl_off_t>(data.size()));
		curl_easy_setopt(easy, CURLOPT_WRITEDATA, &response);
		curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, +[](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t
		{
			http_response* response{ static_cast<http_response*>(userdata) };
			response->write_content(ptr, size * nmemb);
			return size * nmemb;
		});
		if (curl_easy_perform(easy) == CURLE_OK)
		{
			response.set_info_from_easy(easy);
		}
		curl_easy_cleanup(easy);
		curl_slist_free_all(headers);
		return response;
	}

	http_response http_service::post(const std::string& url, const std::filesystem::path& file_path) const
	{
		if (url.empty() || !std::filesystem::exists(file_path))
		{
			return {};
		}
		CURL* easy{ curl_easy_init() };
		if (easy == nullptr)
		{
			return {};
		}
		std::ifstream in{ file_path, std::ios::binary };
		if (!in.is_open())
		{
			curl_easy_cleanup(easy);
			return {};
		}
		curl_slist* headers{ nullptr };
		headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
		http_response response;
		curl_easy_setopt(easy, CURLOPT_URL, url.c_str());
		if (!m_ssl_verification)
		{
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYHOST, 0L);
		}
		curl_easy_setopt(easy, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(easy, CURLOPT_USERAGENT, get_user_agent().c_str());
		curl_easy_setopt(easy, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(easy, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(easy, CURLOPT_POST, 1L);
		curl_easy_setopt(easy, CURLOPT_POSTFIELDSIZE_LARGE, static_cast<curl_off_t>(std::filesystem::file_size(file_path)));
		curl_easy_setopt(easy, CURLOPT_READDATA, &in);
		curl_easy_setopt(easy, CURLOPT_READFUNCTION, +[](char* buffer, size_t size, size_t nmemb, void* userdata) -> size_t
		{
			std::ifstream* in{ static_cast<std::ifstream*>(userdata) };
			in->read(buffer, static_cast<std::streamsize>(size * nmemb));
			return static_cast<size_t>(in->gcount());
		});
		curl_easy_setopt(easy, CURLOPT_WRITEDATA, &response);
		curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, +[](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t
		{
			http_response* response{ static_cast<http_response*>(userdata) };
			response->write_content(ptr, size * nmemb);
			return size * nmemb;
		});
		if (curl_easy_perform(easy) == CURLE_OK)
		{
			response.set_info_from_easy(easy);
		}
		curl_easy_cleanup(easy);
		curl_slist_free_all(headers);
		return response;
	}

	http_response http_service::post(const std::string& url, const std::vector<std::pair<std::string, std::string>>& form) const
	{
		if (url.empty())
		{
			return {};
		}
		CURL* easy{ curl_easy_init() };
		if (easy == nullptr)
		{
			return {};
		}
		curl_mime* mime{ curl_mime_init(easy) };
		for (const std::pair<std::string, std::string>& pair : form)
		{
			curl_mimepart* part{ curl_mime_addpart(mime) };
			curl_mime_name(part, pair.first.c_str());
			curl_mime_data(part, pair.second.c_str(), CURL_ZERO_TERMINATED);
		}
		http_response response;
		curl_easy_setopt(easy, CURLOPT_URL, url.c_str());
		if (!m_ssl_verification)
		{
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYHOST, 0L);
		}
		curl_easy_setopt(easy, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(easy, CURLOPT_USERAGENT, get_user_agent().c_str());
		curl_easy_setopt(easy, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(easy, CURLOPT_MIMEPOST, mime);
		curl_easy_setopt(easy, CURLOPT_WRITEDATA, &response);
		curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, +[](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t
		{
			http_response* response{ static_cast<http_response*>(userdata) };
			response->write_content(ptr, size * nmemb);
			return size * nmemb;
		});
		if (curl_easy_perform(easy) == CURLE_OK)
		{
			response.set_info_from_easy(easy);
		}
		curl_easy_cleanup(easy);
		curl_mime_free(mime);
		return response;
	}

	http_response http_service::put(const std::string& url, const nlohmann::json& json) const
	{
		if (url.empty())
		{
			return {};
		}
		CURL* easy{ curl_easy_init() };
		if (easy == nullptr)
		{
			return {};
		}
		const std::string body{ json.dump() };
		curl_slist* headers{ nullptr };
		headers = curl_slist_append(headers, "Content-Type: application/json");
		http_response response;
		curl_easy_setopt(easy, CURLOPT_URL, url.c_str());
		if (!m_ssl_verification)
		{
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYHOST, 0L);
		}
		curl_easy_setopt(easy, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(easy, CURLOPT_USERAGENT, get_user_agent().c_str());
		curl_easy_setopt(easy, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(easy, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(easy, CURLOPT_CUSTOMREQUEST, "PUT");
		curl_easy_setopt(easy, CURLOPT_POSTFIELDS, body.c_str());
		curl_easy_setopt(easy, CURLOPT_POSTFIELDSIZE, static_cast<curl_off_t>(body.size()));
		curl_easy_setopt(easy, CURLOPT_WRITEDATA, &response);
		curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, +[](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t
		{
			http_response* response{ static_cast<http_response*>(userdata) };
			response->write_content(ptr, size * nmemb);
			return size * nmemb;
		});
		if (curl_easy_perform(easy) == CURLE_OK)
		{
			response.set_info_from_easy(easy);
		}
		curl_easy_cleanup(easy);
		curl_slist_free_all(headers);
		return response;
	}

	http_response http_service::put(const std::string& url, const std::vector<std::byte>& data) const
	{
		if (url.empty())
		{
			return {};
		}
		CURL* easy{ curl_easy_init() };
		if (easy == nullptr)
		{
			return {};
		}
		curl_slist* headers{ nullptr };
		headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
		http_response response;
		curl_easy_setopt(easy, CURLOPT_URL, url.c_str());
		if (!m_ssl_verification)
		{
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYHOST, 0L);
		}
		curl_easy_setopt(easy, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(easy, CURLOPT_USERAGENT, get_user_agent().c_str());
		curl_easy_setopt(easy, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(easy, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(easy, CURLOPT_CUSTOMREQUEST, "PUT");
		curl_easy_setopt(easy, CURLOPT_POSTFIELDS, reinterpret_cast<const char*>(data.data()));
		curl_easy_setopt(easy, CURLOPT_POSTFIELDSIZE, static_cast<curl_off_t>(data.size()));
		curl_easy_setopt(easy, CURLOPT_WRITEDATA, &response);
		curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, +[](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t
		{
			http_response* response{ static_cast<http_response*>(userdata) };
			response->write_content(ptr, size * nmemb);
			return size * nmemb;
		});
		if (curl_easy_perform(easy) == CURLE_OK)
		{
			response.set_info_from_easy(easy);
		}
		curl_easy_cleanup(easy);
		curl_slist_free_all(headers);
		return response;
	}

	http_response http_service::put(const std::string& url, const std::filesystem::path& file_path) const
	{
		if (url.empty() || !std::filesystem::exists(file_path))
		{
			return {};
		}
		CURL* easy{ curl_easy_init() };
		if (easy == nullptr)
		{
			return {};
		}
		std::ifstream in{ file_path, std::ios::binary };
		if (!in.is_open())
		{
			curl_easy_cleanup(easy);
			return {};
		}
		curl_slist* headers{ nullptr };
		headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
		http_response response;
		curl_easy_setopt(easy, CURLOPT_URL, url.c_str());
		if (!m_ssl_verification)
		{
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYHOST, 0L);
		}
		curl_easy_setopt(easy, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(easy, CURLOPT_USERAGENT, get_user_agent().c_str());
		curl_easy_setopt(easy, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(easy, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(easy, CURLOPT_UPLOAD, 1L);
		curl_easy_setopt(easy, CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t>(std::filesystem::file_size(file_path)));
		curl_easy_setopt(easy, CURLOPT_READDATA, &in);
		curl_easy_setopt(easy, CURLOPT_READFUNCTION, +[](char* buffer, size_t size, size_t nmemb, void* userdata) -> size_t
		{
			std::ifstream* in{ static_cast<std::ifstream*>(userdata) };
			in->read(buffer, static_cast<std::streamsize>(size * nmemb));
			return static_cast<size_t>(in->gcount());
		});
		curl_easy_setopt(easy, CURLOPT_WRITEDATA, &response);
		curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, +[](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t
		{
			http_response* response{ static_cast<http_response*>(userdata) };
			response->write_content(ptr, size * nmemb);
			return size * nmemb;
		});
		if (curl_easy_perform(easy) == CURLE_OK)
		{
			response.set_info_from_easy(easy);
		}
		curl_easy_cleanup(easy);
		curl_slist_free_all(headers);
		return response;
	}

	http_response http_service::put(const std::string& url, const std::vector<std::pair<std::string, std::string>>& form) const
	{
		if (url.empty())
		{
			return {};
		}
		CURL* easy{ curl_easy_init() };
		if (easy == nullptr)
		{
			return {};
		}
		curl_mime* mime{ curl_mime_init(easy) };
		for (const std::pair<std::string, std::string>& pair : form)
		{
			curl_mimepart* part{ curl_mime_addpart(mime) };
			curl_mime_name(part, pair.first.c_str());
			curl_mime_data(part, pair.second.c_str(), CURL_ZERO_TERMINATED);
		}
		http_response response;
		curl_easy_setopt(easy, CURLOPT_URL, url.c_str());
		if (!m_ssl_verification)
		{
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(easy, CURLOPT_SSL_VERIFYHOST, 0L);
		}
		curl_easy_setopt(easy, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(easy, CURLOPT_USERAGENT, get_user_agent().c_str());
		curl_easy_setopt(easy, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(easy, CURLOPT_CUSTOMREQUEST, "PUT");
		curl_easy_setopt(easy, CURLOPT_MIMEPOST, mime);
		curl_easy_setopt(easy, CURLOPT_WRITEDATA, &response);
		curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, +[](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t
		{
			http_response* response{ static_cast<http_response*>(userdata) };
			response->write_content(ptr, size * nmemb);
			return size * nmemb;
		});
		if (curl_easy_perform(easy) == CURLE_OK)
		{
			response.set_info_from_easy(easy);
		}
		curl_easy_cleanup(easy);
		curl_mime_free(mime);
		return response;
	}
}