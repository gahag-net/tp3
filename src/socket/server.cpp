#include "server.hpp"

#include <cerrno>
#include <system_error>

#include <sys/socket.h>


tp3::socket::server::server(class addr&& address, uint32_t queue_size)
	: sock(
	  	std::move(address),
	  	tp3::socket::sock::bind
	  )
{
	// http://man7.org/linux/man-pages/man2/listen.2.html
	if (::listen(this->fd, queue_size) < 0)
		throw std::system_error(errno, std::generic_category());
}
