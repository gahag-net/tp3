#pragma once

#include "addr.hpp"


namespace tp3::socket {
	// A basic socket. The derivations of this class provide the useful communication methods.
	class sock {
	protected:
		int fd; // The socket's file descriptor, or -1 when deleted.
		addr _address;

		sock(int fd);
		sock(int fd, addr&& address);

		bool deleted() const noexcept;

	public:
		// Construct with an attach function.
		// The function is called right after construction to attach this socket.
		sock(addr&&, int (&attach)(const sock&));
		sock(const sock&) = delete;
		sock(sock&&);
		~sock();

		sock& operator=(const sock&) = delete;
		sock& operator=(sock&&);

		int descriptor() const noexcept;
		const addr& address() const noexcept;

		bool is_tcp() const noexcept;
		bool is_udp() const noexcept;


		// Common attach functions:

		// Bind for server sockets.
		static int bind(const sock&);
		// Connect for TCP client sockets.
		static int connect(const sock&);
		// A no-op function for UDP sockets.
		static int push(const sock&);
	};
}
