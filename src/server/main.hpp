#pragma once

#include <cstddef>
#include <exception>
#include <system_error>

#include <socket/addr.hpp>


namespace tp3::server::main {
	struct args {
		tp3::socket::addr address;
	};

	args parse_args(int argc, char** argv);


	void sig_handler(int signal, void (*handler)(int));
	int interrupted();
	int exception(const std::exception&);
	int sys_error(const std::system_error&);
}
