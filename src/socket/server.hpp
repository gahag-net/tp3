#pragma once


#include "addr.hpp"
#include "sock.hpp"


namespace tp3::socket {
	// A TCP server socket.
	class server : public sock {
	public:
		server(class addr&&, uint32_t queue_size);
	};
}
