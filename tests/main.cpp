#include <libdesktop.h>
#include "console_lifetime_service.h"

using namespace desktop;

int main(int argc, char* argv[])
{
	hosting::host_options options{ argc, argv };
	options.set_log_path("app.log");
	hosting::host host{ options };
	host.services()->add_service<hosting::lifetime_service, console_lifetime_service>(services::service_scope::singleton);
	host.run();
	return 0;
}