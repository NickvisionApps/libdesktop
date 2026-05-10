#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include "events/event.h"
#include "events/param_event_args.h"
#include "process_status.h"

namespace desktop::system
{
	class process
	{
	public:
		process(std::filesystem::path path, std::vector<std::string> arguments = {});
		~process();
		process(const process&) = delete;
		process(process&&) noexcept = default;
		const events::event<process, events::param_event_args<int>>& get_exited_event() const;
		const events::event<process, events::param_event_args<std::string>>& get_output_received_event() const;
		const events::event<process, events::param_event_args<std::string>>& get_error_received_event() const;
		const std::vector<std::string>& get_arguments() const;
		int get_exit_code() const;
		const std::filesystem::path& get_path() const;
		const std::string& get_standard_error() const;
		const std::string& get_standard_output() const;
		process_status get_status() const;
		const std::filesystem::path& get_working_directory() const;
		bool write(std::string_view data) const;
		bool write_line(const std::string& data) const;
		bool kill();
		bool pause();
		bool resume();
		bool set_working_directory(const std::filesystem::path& path);
		bool start();
		int wait_for_exit() const;
		process& operator=(const process&) = delete;
		process& operator=(process&&) noexcept = default;

	private:
		class impl;
		friend class impl;
		std::unique_ptr<impl> m_impl;
		std::filesystem::path m_path;
		std::vector<std::string> m_arguments;
		std::filesystem::path m_working_directory;
		events::event<process, events::param_event_args<int>> m_exited_event;
		events::event<process, events::param_event_args<std::string>> m_output_received_event;
		events::event<process, events::param_event_args<std::string>> m_error_received_event;
	};
}
