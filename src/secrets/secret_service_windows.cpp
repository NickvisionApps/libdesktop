#include "secrets/secret_service.h"
#include <windows.h>
#include <wincred.h>
#include "helpers/string_manip.h"
#include "secrets/password_generator.h"

using namespace desktop::helpers;

namespace desktop::secrets
{
	std::optional<secret> secret_service::get(const std::string& name) const
	{
		std::wstring wname{ string_manip::wstr(name) };
		CREDENTIALW* cred{ nullptr };
		if (!CredReadW(wname.c_str(), CRED_TYPE_GENERIC, 0, &cred))
		{
			return std::nullopt;
		}
		if (!cred->CredentialBlob || cred->CredentialBlobSize == 0)
		{
			CredFree(cred);
			return std::nullopt;
		}
		std::wstring wvalue{ reinterpret_cast<const wchar_t*>(cred->CredentialBlob), cred->CredentialBlobSize / sizeof(wchar_t) };
		secret s{ name, string_manip::str(wvalue) };
		CredFree(cred);
		return s;
	}

	std::optional<secret> secret_service::create(const std::string& name)
	{
		password_generator gen;
		secret s{ name, gen.generate(64) };
		if (add(s))
		{
			return s;
		}
		return std::nullopt;
	}

	bool secret_service::add(const secret& s)
	{
		if (s.get_value().empty())
		{
			return false;
		}
		std::wstring wname{ string_manip::wstr(s.get_name()) };
		std::wstring wvalue{ string_manip::wstr(s.get_value()) };
		CREDENTIALW cred{ 0 };
		cred.Type = CRED_TYPE_GENERIC;
		cred.Persist = CRED_PERSIST_LOCAL_MACHINE;
		cred.TargetName = LPWSTR(wname.c_str());
		cred.CredentialBlobSize = static_cast<DWORD>(wvalue.size() * sizeof(wchar_t));
		cred.CredentialBlob = LPBYTE(wvalue.c_str());
		return CredWriteW(&cred, 0);
	}

	bool secret_service::update(const secret& s)
	{
		if (s.get_value().empty())
		{
			return false;
		}
		std::wstring wname{ string_manip::wstr(s.get_name()) };
		CREDENTIALW* existing{ nullptr };
		if (!CredReadW(wname.c_str(), CRED_TYPE_GENERIC, 0, &existing))
		{
			return false;
		}
		std::wstring wvalue{ string_manip::wstr(s.get_value()) };
		existing->CredentialBlobSize = static_cast<DWORD>(wvalue.size() * sizeof(wchar_t));
		existing->CredentialBlob = LPBYTE(wvalue.c_str());
		bool res{ static_cast<bool>(CredWriteW(existing, 0)) };
		CredFree(existing);
		return res;
	}

	bool secret_service::remove(const std::string& name)
	{
		std::wstring wname{ string_manip::wstr(name) };
		return CredDeleteW(wname.c_str(), CRED_TYPE_GENERIC, 0);
	}
}
