#include "secrets/secret_service.h"
#include <libsecret/secret.h>
#include "secrets/password_generator.h"

static const SecretSchema LIBDESKTOP_SCHEMA =
{
	"libdesktop",
	SECRET_SCHEMA_DONT_MATCH_NAME,
	{
		{ "application", SECRET_SCHEMA_ATTRIBUTE_STRING },
		{ "NULL", SECRET_SCHEMA_ATTRIBUTE_STRING }
	}
};

namespace desktop::secrets
{
	std::optional<secret> secret_service::get(const std::string& name) const
	{
		GError* error{ nullptr };
		char* value{ secret_password_lookup_sync(&LIBDESKTOP_SCHEMA, nullptr, &error, "application", name.c_str(), nullptr) };
		if(error)
		{
			g_error_free(error);
			return std::nullopt;
		}
		if(!value)
		{
			return std::nullopt;
		}
		secret s{ name, value };
		secret_password_free(value);
		return s;
	}

	std::optional<secret> secret_service::create(const std::string& name)
	{
		password_generator gen;
		secret s{ name, gen.next(64) };
		if(add(s))
		{
			return s;
		}
		return std::nullopt;
	}

	bool secret_service::add(const secret& s)
	{
		if(s.get_value().empty())
		{
			return false;
		}
		GError* error{ nullptr };
		secret_password_store_sync(&LIBDESKTOP_SCHEMA, SECRET_COLLECTION_DEFAULT, s.get_name().c_str(), s.get_value().c_str(), nullptr, &error, "application", s.get_name().c_str(), nullptr);
		if(error)
		{
			g_error_free(error);
			return false;
		}
		return true;
	}

	bool secret_service::update(const secret& s)
	{
		if(s.get_value().empty())
		{
			return false;
		}
		GError* error{ nullptr };
		char* existing{ secret_password_lookup_sync(&LIBDESKTOP_SCHEMA, nullptr, &error, "application", s.get_name().c_str(), nullptr) };
		if(error)
		{
			g_error_free(error);
			return false;
		}
		if(!existing)
		{
			return false;
		}
		secret_password_free(existing);
		secret_password_store_sync(&LIBDESKTOP_SCHEMA, SECRET_COLLECTION_DEFAULT, s.get_name().c_str(), s.get_value().c_str(), nullptr, &error, "application", s.get_name().c_str(), nullptr);
		if(error)
		{
			g_error_free(error);
			return false;
		}
		return true;
	}

	bool secret_service::remove(const std::string& name)
	{
		GError* error{ nullptr };
		bool res{ static_cast<bool>(secret_password_clear_sync(&LIBDESKTOP_SCHEMA, nullptr, &error, "application", name.c_str(), nullptr)) };
		if(error)
		{
			g_error_free(error);
			return false;
		}
		return res;
	}
}
