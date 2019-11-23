#pragma once

#include <socket/addr.hpp>


namespace tp3::client::main {
	struct args {
		tp3::socket::addr address;
	};

	args parse_args(int argc, char** argv);

	int main(int argc, char* argv[]);
}
