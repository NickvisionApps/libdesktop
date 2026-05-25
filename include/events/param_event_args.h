#pragma once

#include <utility>
#include "event_args.h"

namespace desktop::events
{
	template <typename T>
	class param_event_args : public event_args
	{
	public:
		param_event_args(T value)
		    : m_value{ std::move(value) }
		{
		}
		~param_event_args() override = default;
		param_event_args(const param_event_args&) = default;
		param_event_args(param_event_args&&) noexcept = default;
		T get_value() const
		{
			return m_value;
		}
		T operator*() const
		{
			return m_value;
		}
		param_event_args& operator=(const param_event_args&) = default;
		param_event_args& operator=(param_event_args&&) noexcept = default;

	private:
		T m_value;
	};

	template <typename T>
	    requires std::is_class_v<T>
	class param_event_args<T> : public event_args
	{
	public:
		param_event_args(T value)
		    : m_value{ std::move(value) }
		{
		}
		~param_event_args() override = default;
		param_event_args(const param_event_args&) = default;
		param_event_args(param_event_args&&) noexcept = default;
		const T& get_value() const
		{
			return m_value;
		}
		const T& operator*() const
		{
			return m_value;
		}
		const T* operator->() const
		{
			return &m_value;
		}
		param_event_args& operator=(const param_event_args&) = default;
		param_event_args& operator=(param_event_args&&) noexcept = default;

	private:
		T m_value;
	};
}