#pragma once

#include <filesystem>
#include <memory>
#include <span>
#include "app/app_info.h"

namespace desktop::hosting
{
	class host_options
	{
	public:
		host_options(std::shared_ptr<app::app_info> app_info, std::span<char*> argv);
		~host_options() = default;
		host_options(const host_options&) = default;
		host_options(host_options&&) noexcept = default;
		const std::shared_ptr<app::app_info>& get_app_info() const;
		std::span<char*> get_argv() const;
		bool is_single_instance() const;
		void set_single_instance(bool single);
		const std::filesystem::path& get_log_path() const;
		void set_log_path(const std::filesystem::path& log_path);
		host_options& operator=(const host_options&) = default;
		host_options& operator=(host_options&&) noexcept = default;

	private:
		std::shared_ptr<app::app_info> m_app_info;
		std::span<char*> m_argv;
		bool m_single_instance;
		std::filesystem::path m_log_path;
	};
}