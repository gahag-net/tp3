#include "sock.hpp"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <system_error>

#include <sys/socket.h>
#include <unistd.h>


tp3::socket::sock::sock(const int fd) : fd(fd), _address(fd) {
	if (fd < 0)
		throw std::system_error(errno, std::generic_category());
}

tp3::socket::sock::sock(
	const int fd,
	tp3::socket::addr&& address
) : fd(fd),
		_address(std::move(address))
{
	if (fd < 0)
		throw std::system_error(errno, std::generic_category());
}

tp3::socket::sock::sock(
	tp3::socket::addr&& address,
	int (&attach)(const sock&)
) : sock(
	  	// http://man7.org/linux/man-pages/man2/socket.2.html
	  	::socket(address->ai_family, address->ai_socktype, address->ai_protocol),
	  	std::move(address)
	  )
{
	if (attach(*this) < 0)
		throw std::system_error(errno, std::generic_category());
}

tp3::socket::sock::sock(sock&& other) : fd(other.fd), _address(std::move(other._address)) {
	other.fd = -1; // mark other as deleted.
}

tp3::socket::sock::~sock() {
	if (this->deleted())
		return;

	// http://man7.org/linux/man-pages/man2/close.2.html
	if (::close(this->fd) < 0) {
		std::cerr << "Failed to close connection (fd = " << this->fd << "):"
		          << std::endl
		          << std::strerror(errno);
	}

	this->fd = -1;
}


tp3::socket::sock& tp3::socket::sock::operator=(sock&& other) {
	this->~sock();

	this->fd = other.fd;
	other.fd = -1; // mark other as deleted.

	this->_address = std::move(other._address);

	return *this;
}


bool tp3::socket::sock::deleted() const noexcept {
	return this->fd == -1;
}


int tp3::socket::sock::descriptor() const noexcept {
	return this->fd;
}

const tp3::socket::addr& tp3::socket::sock::address() const noexcept {
	return this->_address;
}


bool tp3::socket::sock::is_tcp() const noexcept {
	return this->_address->ai_socktype == SOCK_STREAM;
}

bool tp3::socket::sock::is_udp() const noexcept {
	return this->_address->ai_socktype == SOCK_DGRAM;
}


int tp3::socket::sock::bind(const sock& sock) {
	const auto fd = sock.descriptor();
	const auto& addr = sock.address();

	int on = 1;
	// allow address reuse if possible:
	// http://man7.org/linux/man-pages/man2/setsockopt.2.html
	if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
		throw std::system_error(errno, std::generic_category());

	// http://man7.org/linux/man-pages/man2/bind.2.html
	return ::bind(fd, addr->ai_addr, addr->ai_addrlen);
}

int tp3::socket::sock::connect(const sock& sock) {
	if (!sock.is_tcp())
		throw std::invalid_argument("Call to tcp connect using udp address");

	const auto fd = sock.descriptor();
	const auto& addr = sock.address();

	// http://man7.org/linux/man-pages/man2/connect.2.html
	return ::connect(fd, addr->ai_addr, addr->ai_addrlen);
}

int tp3::socket::sock::push(const sock& sock) {
	if (!sock.is_udp())
		throw std::invalid_argument("Call to udp socket using tcp address");

	return 0;
}
