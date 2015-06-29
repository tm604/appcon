#include <memory>
#include <appcon/detail.h>

std::shared_ptr<appcon::config>
make_config()
{
	auto cfg = std::static_pointer_cast<appcon::config>(
		std::make_shared<appcon::detail::config>()
	);
	return cfg;
}


