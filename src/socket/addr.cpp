#include "addr.hpp"

#include <cerrno>


tp3::socket::name::name(std::string&& node, std::string&& service)
	: node(std::move(node)),
	  service(std::move(service))
{ }

tp3::socket::name::name(struct sockaddr* addr, socklen_t size) {
	const size_t max_size = 256;

	char node[max_size];
	char service[max_size];

	// http://man7.org/linux/man-pages/man3/getnameinfo.3.html
	const auto result = ::getnameinfo(
		addr,
		size,
		node,
		sizeof(node),
		service,
		sizeof(service),
		0
	);

	if (result != 0)
		throw addr::eai_exception(result);

	this->node = std::string(node);
	this->service = std::string(service);
}



std::system_error tp3::socket::addr::eai_exception(int eai_error) {
	if (eai_error == EAI_SYSTEM)
		return std::system_error(
			errno,
			std::generic_category()
		);
	else
		return std::system_error(
			std::error_code(
				eai_error,
				std::generic_category()
			),
			// http://man7.org/linux/man-pages/man3/gai_strerror.3p.html
			::gai_strerror(eai_error)
		);
}


tp3::socket::addr::addr(
	const int fd,
	const std::function<int(int, sockaddr*, socklen_t*)>& filladdr
) {
	struct sockaddr_in6 address; // sockaddr_in6 is the biggest of sockaddrs.

	// filladdr:
	socklen_t size = sizeof(address);
	socklen_t original_size = size;

	const auto sockaddr = reinterpret_cast<struct sockaddr *>(&address);

	if (filladdr(fd, sockaddr, &size) < 0)
		throw std::system_error(errno, std::generic_category());

	if (size > original_size)
		throw std::length_error(
			std::string(
				typeid(address).name()
			)
			.append(" too small")
		);

	// name:
	const name name(sockaddr, size);

	// addrinfo hint:
	addrinfo addrinfo_hint {
		.ai_family = sockaddr->sa_family
	};
	size = sizeof(addrinfo_hint.ai_socktype);
	original_size = size;

	if (::getsockopt(fd, SOL_SOCKET, SO_TYPE, &addrinfo_hint.ai_socktype, &size) < 0)
		throw std::system_error(errno, std::generic_category());

	if (size > original_size)
		throw std::length_error("socktype too small");

	// addrinfo:
	// http://man7.org/linux/man-pages/man3/getaddrinfo.3.html
	auto result = ::getaddrinfo(
		name.node.c_str(),
		name.service.c_str(),
		&addrinfo_hint,
		&this->data
	);

	if (result != 0)
		throw addr::eai_exception(result);
}

tp3::socket::addr::addr(const int fd)
	: addr(
	  	fd,
	  	[](const int fd, sockaddr* const addr, socklen_t* const size) {
	  		if (fd < 0)
	  			throw std::system_error(errno, std::generic_category());

				// http://man7.org/linux/man-pages/man2/getpeername.2.html
	  		return ::getpeername(fd, addr, size);
	  	}
	  )
{ }

tp3::socket::addr::addr(const name& name, const addrinfo& hints) {
	// http://man7.org/linux/man-pages/man3/getaddrinfo.3.html
	const auto result = ::getaddrinfo(
		name.node.c_str(),
		name.service.c_str(),
		&hints,
		&this->data
	);

	if (result != 0)
		throw addr::eai_exception(result);
}

tp3::socket::addr::addr(addr&& other) : data(other.data) {
	other.data = nullptr;
}

tp3::socket::addr::~addr() {
	if (this->data != nullptr)
		// http://man7.org/linux/man-pages/man3/freeaddrinfo.3p.html
		::freeaddrinfo(this->data);
}


tp3::socket::addr& tp3::socket::addr::operator=(addr&& other) {
	this->~addr();

	this->data = other.data;
	other.data = nullptr;

	return *this;
}


addrinfo& tp3::socket::addr::operator*() {
	return *this->data;
}

addrinfo* tp3::socket::addr::operator->() {
	return this->data;
}

const addrinfo& tp3::socket::addr::operator*() const {
	return *this->data;
}

const addrinfo* tp3::socket::addr::operator->() const {
	return this->data;
}


std::ostream& operator<<(std::ostream& stream, const tp3::socket::addr& address) {
	const tp3::socket::name name(address->ai_addr, address->ai_addrlen);

  stream << name.node << ':' << name.service;

	switch (address->ai_family) {
		case AF_INET: stream << " (IPv4"; break;
		case AF_INET6: stream << " (IPv6"; break;
		default: stream << " (Unknown"; break;
	}

	switch (address->ai_socktype) {
		case SOCK_STREAM: stream << ", TCP)"; break;
		case SOCK_DGRAM: stream << ", UDP)"; break;
		default: stream << ", RAW)"; break;
	}

	return stream;
}
