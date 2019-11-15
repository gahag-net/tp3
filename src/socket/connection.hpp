#pragma once

#include <cstdint>
#include <memory>

#include "addr.hpp"
#include "server.hpp"
#include "sock.hpp"


namespace tp3::socket {
	// A TCP connection socket.
	class connection : public sock {
	public:
		// Connect to an address.
		connection(class addr&&);
		// Accept connection from TCP server.
		connection(const server&);
		~connection();

		std::unique_ptr<uint8_t[]> recv(std::size_t&) const;
		std::size_t send(const uint8_t[], std::size_t) const;
	};
}
