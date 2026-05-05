#pragma once

namespace desktop::events
{
	class event_args
	{
	public:
		event_args() = default;
		virtual ~event_args() = default;
		event_args(const event_args&) = default;
		event_args(event_args&&) noexcept = default;
		event_args& operator=(const event_args&) = default;
		event_args& operator=(event_args&&) noexcept = default;
	};
}