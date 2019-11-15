#pragma once

#include <cstdint>
#include <memory>
#include <tuple>

#include "addr.hpp"
#include "sock.hpp"


namespace tp3::socket {
	// A UDP socket
	class push : public sock {
	public:
		// attach must be bind for a server, or push for a client.
		push(class addr&&, int (&attach)(const sock&));

		std::tuple<std::unique_ptr<uint8_t[]>, class addr> recv(std::size_t&) const;
		std::size_t send(const uint8_t[], std::size_t, const class addr&) const;
	};
}
