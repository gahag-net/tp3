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

		connection(const connection&) = delete;
		connection(connection&&) = default;
		connection& operator=(const connection&) = delete;
		connection& operator=(connection&&) = default;

		~connection();

		bool is_closed() const;

		std::size_t recv(uint8_t[], std::size_t) const;
		std::size_t send(const uint8_t[], std::size_t) const;
	};
}
