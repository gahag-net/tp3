#pragma once

#include <functional>
#include <ostream>
#include <string>
#include <system_error>

#include <netdb.h>
#include <sys/socket.h>


namespace tp3::socket {
	// A node:service pair.
	class name {
	public:
		std::string node;
		std::string service;

		name() = default;
		name(std::string&& node, std::string&& service);
		name(struct sockaddr* addr, socklen_t size);
		~name() = default;
	};


	// A socket address structure.
	class addr {
	protected:
		addrinfo* data; // nullptr when deleted.

	public:
		static std::system_error eai_exception(int eai_error);

		// Construct from a file descriptor.
		addr(int fd);
		// Construct from a file descriptor and a function to fill a sockaddr.
		addr(int fd, const std::function<int(int, sockaddr*, socklen_t*)>& fill_addr);
		// Construct from a name.
		addr(const name& nameinfo, const addrinfo& hints);
		addr(const addr&) = delete;
		addr(addr&&);
		~addr();

		addr& operator=(const addr&) = delete;
		addr& operator=(addr&&);

		addrinfo& operator*();
		addrinfo* operator->();

		const addrinfo& operator*() const;
		const addrinfo* operator->() const;
	};


	std::ostream& operator<<(std::ostream& stream, const addr& address);
}
