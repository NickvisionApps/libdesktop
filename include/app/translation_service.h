#pragma once

#include "services/service.h"

namespace desktop::app
{
	class translation_service : public services::service
	{
	public:
		translation_service() = default;
		~translation_service() override = default;
		translation_service(const translation_service&) = delete;
		translation_service(translation_service&&) = delete;
		translation_service& operator=(const translation_service&) = delete;
		translation_service& operator=(translation_service&&) = delete;
	};
}