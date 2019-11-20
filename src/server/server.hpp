#pragma once

#include <poll.h>

#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <iterator>
#include <system_error>
#include <vector>

#include <socket/server.hpp>
#include <socket/connection.hpp>
#include <server/client.hpp>
#include <util/algorithm.hpp>
#include <util/boxed_array.hpp>
#include <util/overload.hpp>


namespace tp3::server {
	template<typename T>
	using boxed_array = tp3::util::boxed_array<T>;


	template<std::size_t client_buffer_size>
	class server {
	protected:
		tp3::socket::server socket;

		std::vector<client<client_buffer_size>> clients;

		std::vector<pollfd> poll_sockets;

		std::unordered_map<
			boxed_array<uint8_t>,
			std::size_t
		> catalogue;


		using clients_iter = typename decltype(clients)::iterator;
		using sockets_iter = typename decltype(poll_sockets)::iterator;


	public:
		server(const server&) = delete;
		server(server&&) noexcept = default;

		server(tp3::socket::addr&& address, uint32_t queue_size = 32)
			: socket(std::move(address), queue_size) {
			this->poll_sockets.emplace_back(
				pollfd {
					.fd = this->socket.descriptor(),
					.events = POLLIN
				}
			);
		}


		clients_iter get_client(const sockets_iter& it) {
			auto begin = this->poll_sockets.begin();

			const auto ix = std::distance(begin, it) - 1;

			return this->clients.begin() + ix;
		}


		std::vector<boxed_array<uint8_t>> list_users() {
			std::vector<boxed_array<uint8_t>> usernames;

			usernames.reserve(
				this->clients.size()
				);

			std::transform(
				this->clients.begin(),
				this->clients.end(),
				std::back_inserter(usernames),
				[](const auto& client) {
					return boxed_array<uint8_t>(client.name());
				}
			);

			return usernames;
		}


		// accept new connection.
		void accept() {
			client<client_buffer_size> client(
				tp3::socket::connection(this->socket)
			);

			if (this->catalogue.find(client.name()) != this->catalogue.end()) {
				// TODO: connection refused, name already in use.
			}
			else {
				this->clients.push_back(std::move(client));

				const auto& client = this->clients.back();

				this->poll_sockets.push_back(
					pollfd {
						.fd = client.descriptor(),
						.events = POLLIN
					}
				);

				this->catalogue[client.name()] = this->clients.size() - 1;
			}
		}


		void process_client(client<client_buffer_size>& client) {
			if (const auto message = client.read())
				std::visit(
					tp3::util::overload {
						[&](const message::list_users&) {
							client.send(
								tp3::client::message::users_list(
									this->list_users()
								)
							);
						},

						[&](const message::broadcast&) {
						},

						[&](const message::unicast&) {
						}
					},
					*message
				);
		}


		void process() {
			constexpr int infinite_timeout = -1;

			while (true) {
				auto result = ::poll(
					this->poll_sockets.data(),
					this->poll_sockets.size(),
					infinite_timeout
				);

				if (result < 0)
					throw std::system_error(errno, std::generic_category());

				auto socket = this->poll_sockets.begin();
				auto end = this->poll_sockets.end();

				if (socket->revents | POLLIN) // new client incoming
					this->accept();

				while (socket != end) {
					if (socket->revents | POLLIN) {
						auto client = this->get_client(socket);

						if (!client->connected()) { // client disconnected, remove from list:
							this->catalogue.erase(client->name());
							tp3::util::algorithm::swap_pop(this->poll_sockets, socket);
							tp3::util::algorithm::swap_pop(this->clients, client);

							if (!this->clients.empty())
								// client now refers to the swapped client:
								this->catalogue[client->name()] = client - this->clients.begin();
						}
						else
							this->process_client(*client);
					}

					++socket;
				}
			}
		}
	};
}
