#pragma once

#include <memory>

namespace desktop::app
{
	class ipc_service
	{
	public:
		ipc_service() = default;
		~ipc_service() = default;
		ipc_service(const ipc_service&) = delete;
		ipc_service(ipc_service&&) = delete;
		ipc_service& operator=(const ipc_service&) = delete;
		ipc_service& operator=(ipc_service&&) = delete;

	private:
		class impl;
		friend class impl;
		std::unique_ptr<impl> m_impl;
	};
}