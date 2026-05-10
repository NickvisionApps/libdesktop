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
		std::scoped_lock lock{ m_mutex };
		std::wstring wname{ string_manip::wstr(name) };
		CREDENTIALW* cred{ nullptr };
		if (CredReadW(wname.c_str(), CRED_TYPE_GENERIC, 0, &cred) == FALSE)
		{
			return std::nullopt;
		}
		if (cred->CredentialBlob == nullptr || cred->CredentialBlobSize == 0)
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
		std::scoped_lock lock{ m_mutex };
		if (s.get_value().empty())
		{
			return false;
		}
		std::wstring wname{ string_manip::wstr(s.get_name()) };
		std::wstring wvalue{ string_manip::wstr(s.get_value()) };
		CREDENTIALW cred{ .Type = CRED_TYPE_GENERIC,
			              .TargetName = wname.data(),
			              .CredentialBlobSize = static_cast<DWORD>(wvalue.size() * sizeof(wchar_t)),
			              .CredentialBlob = reinterpret_cast<LPBYTE>(wvalue.data()),
			              .Persist = CRED_PERSIST_LOCAL_MACHINE };
		return CredWriteW(&cred, 0) != FALSE;
	}

	bool secret_service::update(const secret& s)
	{
		std::scoped_lock lock{ m_mutex };
		if (s.get_value().empty())
		{
			return false;
		}
		std::wstring wname{ string_manip::wstr(s.get_name()) };
		CREDENTIALW* existing{ nullptr };
		if (CredReadW(wname.c_str(), CRED_TYPE_GENERIC, 0, &existing) == FALSE)
		{
			return false;
		}
		std::wstring wvalue{ string_manip::wstr(s.get_value()) };
		existing->CredentialBlobSize = static_cast<DWORD>(wvalue.size() * sizeof(wchar_t));
		existing->CredentialBlob = reinterpret_cast<LPBYTE>(wvalue.data());
		bool res{ CredWriteW(existing, 0) != FALSE };
		CredFree(existing);
		return res;
	}

	bool secret_service::remove(const std::string& name)
	{
		std::scoped_lock lock{ m_mutex };
		std::wstring wname{ string_manip::wstr(name) };
		return CredDeleteW(wname.c_str(), CRED_TYPE_GENERIC, 0) != FALSE;
	}
}
