#include "secrets/secret_service.h"
#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#include "secrets/password_generator.h"

namespace desktop::secrets
{
	std::optional<secret> secret_service::get(const std::string& name) const
	{
		CFStringRef name_ref{ CFStringCreateWithCString(nullptr, name.c_str(), kCFStringEncodingUTF8) };
		CFMutableDictionaryRef query{ CFDictionaryCreateMutable(nullptr, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks) };
		CFDictionaryAddValue(query, kSecClass, kSecClassGenericPassword);
		CFDictionaryAddValue(query, kSecAttrService, name_ref);
		CFDictionaryAddValue(query, kSecMatchLimit, kSecMatchLimitOne);
		CFDictionaryAddValue(query, kSecReturnData, kCFBooleanTrue);
		CFTypeRef result{ nullptr };
		OSStatus status{ SecItemCopyMatching(query, &result) };
		CFRelease(query);
		CFRelease(name_ref);
		if(status != errSecSuccess || !result)
		{
			return std::nullopt;
		}
		CFDataRef data{ reinterpret_cast<CFDataRef>(result) };
		std::string value{ reinterpret_cast<const char*>(CFDataGetBytePtr(data)), static_cast<std::size_t>(CFDataGetLength(data)) };
		CFRelease(result);
		return secret{ name, value };
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
		if(s.value().empty())
		{
			return false;
		}
		CFStringRef name_ref{ CFStringCreateWithCString(nullptr, s.name().c_str(), kCFStringEncodingUTF8) };
		CFDataRef value_data{ CFDataCreate(nullptr, reinterpret_cast<const UInt8*>(s.value().c_str()), static_cast<CFIndex>(s.value().size())) };
		CFMutableDictionaryRef query{ CFDictionaryCreateMutable(nullptr, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks) };
		CFDictionaryAddValue(query, kSecClass, kSecClassGenericPassword);
		CFDictionaryAddValue(query, kSecAttrService, name_ref);
		CFDictionaryAddValue(query, kSecValueData, value_data);
		CFDictionaryAddValue(query, kSecAttrSynchronizable, kCFBooleanFalse);
		OSStatus status{ SecItemAdd(query, nullptr) };
		CFRelease(query);
		CFRelease(value_data);
		CFRelease(name_ref);
		return status == errSecSuccess;
	}

	bool secret_service::update(const secret& s)
	{
		if(s.value().empty())
		{
			return false;
		}
		CFStringRef name_ref{ CFStringCreateWithCString(nullptr, s.name().c_str(), kCFStringEncodingUTF8) };
		CFMutableDictionaryRef query{ CFDictionaryCreateMutable(nullptr, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks) };
		CFDictionaryAddValue(query, kSecClass, kSecClassGenericPassword);
		CFDictionaryAddValue(query, kSecAttrService, name_ref);
		CFDataRef value_data{ CFDataCreate(nullptr, reinterpret_cast<const UInt8*>(s.value().c_str()), static_cast<CFIndex>(s.value().size())) };
		CFMutableDictionaryRef attrs{ CFDictionaryCreateMutable(nullptr, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks) };
		CFDictionaryAddValue(attrs, kSecValueData, value_data);
		OSStatus status{ SecItemUpdate(query, attrs) };
		CFRelease(attrs);
		CFRelease(value_data);
		CFRelease(query);
		CFRelease(name_ref);
		return status == errSecSuccess;
	}

	bool secret_service::remove(const std::string& name)
	{
		CFStringRef name_ref{ CFStringCreateWithCString(nullptr, name.c_str(), kCFStringEncodingUTF8) };
		CFMutableDictionaryRef query{ CFDictionaryCreateMutable(nullptr, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks) };
		CFDictionaryAddValue(query, kSecClass, kSecClassGenericPassword);
		CFDictionaryAddValue(query, kSecAttrService, name_ref);
		OSStatus status{ SecItemDelete(query) };
		CFRelease(query);
		CFRelease(name_ref);
		return status == errSecSuccess;
	}
}
