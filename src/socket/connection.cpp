#include "connection.hpp"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <system_error>

#include <sys/socket.h>


tp3::socket::connection::connection(class addr&& address)
	: sock(
	  	std::move(address),
	  	sock::connect
	  )
{ }

tp3::socket::connection::connection(const server& server)
	: sock(
	  	// http://man7.org/linux/man-pages/man2/accept.2.html
	  	::accept(server.descriptor(), nullptr, nullptr)
	  )
{ }


tp3::socket::connection::~connection() {
	if (::shutdown(this->fd, SHUT_RDWR) < 0)
		std::cerr << "Failed to close connection (fd = " << this->fd << "):"
		          << std::endl
		          << std::strerror(errno);
}


bool tp3::socket::connection::is_closed() const {
	// http://man7.org/linux/man-pages/man2/recv.2.html
	uint8_t dump;

	auto size = ::recv(
		this->fd,
		&dump,
		1,
		MSG_PEEK
	);

	if (size < 0)
		throw std::system_error(errno, std::generic_category());

	return size == 0;
}

std::size_t tp3::socket::connection::recv(uint8_t buffer[], std::size_t size) const {
	// http://man7.org/linux/man-pages/man2/recv.2.html
	size = ::recv(this->fd, buffer, size, 0);

	if (size < 0)
		throw std::system_error(errno, std::generic_category());

	return size;
}


std::size_t tp3::socket::connection::send(
	const uint8_t buffer[],
	std::size_t size
) const {
	// http://man7.org/linux/man-pages/man2/sendto.2.html
	size = ::send(this->fd, buffer, size, 0);

	if (size < 0)
		throw std::system_error(errno, std::generic_category());

	return size;
}
