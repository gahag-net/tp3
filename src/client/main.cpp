#include "main.hpp"

#include <iostream>

#include <netdb.h>
#include <sys/socket.h>

#include <client/client.hpp>


namespace tp3::client::main {
	args parse_args(int argc, char** argv) {
		if (argc < 3) {
			std::cerr << "Missing arguments" << std::endl;
			std::cerr << "Usage: " << argv[0] << " <ip> <port>" << std::endl;
			::exit(1);
		}

		return (args) {
			.address = tp3::socket::addr(
				tp3::socket::name(argv[1], argv[2]),
				(addrinfo) {
					.ai_family = AF_UNSPEC, // accept both ipv4 and ipv6
					.ai_socktype = SOCK_STREAM // force TCP
				}
			)
		};
	}

	int main(int argc, char* argv[]) try {
		args args = parse_args(argc, argv);

		tp3::client::client<1024> client(
			std::move(args.address)
		);

		client.process();

		return 0;
	}
	catch (const std::exception& e) {
		std::cerr << "Fatal: " << e.what() << std::endl;
		return -1;
	}
}


int main(int argc, char *argv[]) {
	return tp3::client::main::main(argc, argv);
}

