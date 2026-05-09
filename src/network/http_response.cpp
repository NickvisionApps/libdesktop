#include "network/http_response.h"

namespace desktop::network
{
	http_response::http_response()
	    : m_response_code{ 0 },
	      m_elapsed{ 0 }
	{
	}

	bool http_response::is_success() const
	{
		return m_response_code >= 200 && m_response_code < 300;
	}

	long http_response::get_response_code() const
	{
		return m_response_code;
	}

	void http_response::set_response_code(long code)
	{
		m_response_code = code;
	}

	const std::chrono::seconds& http_response::get_elapsed() const
	{
		return m_elapsed;
	}

	void http_response::set_elapsed(const std::chrono::seconds& elapsed)
	{
		m_elapsed = elapsed;
	}

	std::string http_response::get_content_as_string() const
	{
		return { m_content.data(), m_content.size() };
	}

	nlohmann::json http_response::get_content_as_json() const
	{
		try
		{
			return nlohmann::json::parse(get_content_as_string());
		}
		catch (...)
		{
			return {};
		}
	}

	void http_response::write_content(const char* ptr, size_t size)
	{
		m_content.insert(m_content.end(), ptr, ptr + size);
	}

	void http_response::set_info_from_easy(CURL* easy)
	{
		double total_time{ 0.0 };
		curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &m_response_code);
		curl_easy_getinfo(easy, CURLINFO_TOTAL_TIME, &total_time);
		m_elapsed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::duration<double>{ total_time });
	}
}