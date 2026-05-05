#pragma once
#include "event_args.h"

namespace desktop::events
{
	template <typename T>
	class param_event_args : public event_args
	{
	public:
		param_event_args(const T& value)
		    : m_value{ value }
		{
		}
		~param_event_args() override = default;
		param_event_args(const param_event_args& other) = default;
		param_event_args(param_event_args&& other) noexcept = default;
		const T& get_value() const
		{
			return m_value;
		}
		param_event_args& operator=(const param_event_args& other) = default;
		param_event_args& operator=(param_event_args&& other) noexcept = default;

	private:
		T m_value;
	};
}