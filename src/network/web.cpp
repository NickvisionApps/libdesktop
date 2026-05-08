#include "network/web.h"
#include <curl/curl.h>
#include <fstream>

namespace desktop::network
{
	bool web::download_file(const std::string& url, const std::filesystem::path& destination, bool overwrite, const cpr::ProgressCallback& progress)
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
		cpr::Session session;
		session.SetUrl(url);
		if (progress.callback)
		{
			session.SetProgressCallback(progress);
		}
		long status{ session.Download(out).status_code };
		out.close();
		if (status >= 200 && status < 300)
		{
			return true;
		}
		std::filesystem::remove(destination);
		return false;
	}

	nlohmann::json web::get_json(const std::string& url)
	{
		if (url.empty())
		{
			return {};
		}
		cpr::Session session;
		session.SetUrl(url);
		session.SetHeader({ { "Accept", "application/json" } });
#ifdef _WIN32
		session.SetUserAgent(
		    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/147.0.0.0 Safari/537.36 Edg/148.0.3967.54");
#elif defined(__linux__)
		session.SetUserAgent("Mozilla/5.0 (X11; Linux x86_64; rv:150.0) Gecko/20100101 Firefox/150.0");
#elif defined(__APPLE__)
		session.SetUserAgent("Mozilla/5.0 (Macintosh; Intel Mac OS X 15_7_5) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/26.0 Safari/605.1.15");
#endif
		cpr::Response response{ session.Get() };
		if (response.status_code >= 200 && response.status_code < 300 && !response.text.empty())
		{
			try
			{
				return nlohmann::json::parse(response.text);
			}
			catch (...)
			{
				return {};
			}
		}
		return {};
	}

	bool web::is_existing_url(const std::string& url)
	{
		if (url.empty())
		{
			return false;
		}
		long status{ cpr::Head(cpr::Url{ url }, cpr::Timeout{ 5000 }).status_code };
		if (status == 0)
		{
			status = cpr::Get(cpr::Url{ url }, cpr::Timeout{ 5000 }).status_code;
		}
		return status >= 200 && status < 400;
	}

	bool web::is_valid_url(const std::string& url)
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
}