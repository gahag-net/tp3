#include "main.hpp"

#include <algorithm>
#include <iostream>
#include <cerrno>
#include <cstdlib>

#include <netdb.h>
#include <signal.h>
#include <sys/socket.h>

#include <server/server.hpp>


namespace tp3::server::main {
	args parse_args(int argc, char** argv) {
		if (argc < 2) {
			std::cerr << "Missing port argument" << std::endl;
			std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
			::exit(1);
		}

		return (args) {
			.address = tp3::socket::addr(
				tp3::socket::name("::", argv[1]),
				(addrinfo) {
					.ai_family = AF_UNSPEC, // accept both ipv4 and ipv6
					.ai_socktype = SOCK_STREAM // force TCP
				}
			)
		};
	}


	void sig_handler(int signal, void (*handler)(int)) {
		struct sigaction action;
		action.sa_handler = handler;

		if (::sigemptyset(&action.sa_mask) != 0)
			throw std::system_error(errno, std::generic_category());

		if (::sigaction(signal, &action, nullptr) != 0)
			throw std::system_error(errno, std::generic_category());
	}


	int interrupted() {
		std::cerr << "Interrupted!" << std::endl;
		return 1;
	}

	int exception(const std::exception& e) {
		std::cerr << "Fatal: " << e.what() << std::endl;
		return -1;
	}

	int sys_error(const std::system_error& e) {
		return e.code().value() == EINTR
			? interrupted()
			: exception(e);
	}


	int main(int argc, char* argv[]) try {
		sig_handler(
			SIGINT,
			[](int) { }
		);

		args args = parse_args(argc, argv);

		tp3::server::server<1024> server(
			std::move(args.address)
		);

		server.process();

		return interrupted();
	}
	catch (const std::system_error& e) {
		return sys_error(e);
	} catch (const std::exception& e) {
		return exception(e);
	}
}


int main(int argc, char *argv[]) {
	return tp3::server::main::main(argc, argv);
}
