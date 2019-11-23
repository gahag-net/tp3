#pragma once

#include <poll.h>

#include <iostream>
#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <iterator>
#include <system_error>
#include <sstream>
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


	template<std::size_t buffer_size>
	class server {
	protected:
		tp3::socket::server socket;

		std::vector<client<buffer_size>> clients;

		std::vector<pollfd> poll_sockets; // server socket : clients sockets

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
			: socket(std::move(address), queue_size)
		{
			this->poll_sockets.emplace_back(
				pollfd {
					.fd = this->socket.descriptor(),
					.events = POLLIN
				}
			);
		}


		// Get a client iterator from a sockets iterator.
		clients_iter get_client(const sockets_iter& it) {
			auto begin = this->poll_sockets.begin();

			// As the poll_sockets vector starts with the server socket, we must subtract 1.
			const auto ix = std::distance(begin, it) - 1;

			return this->clients.begin() + ix;
		}


		std::vector<boxed_array<uint8_t>> list_users() const {
			std::vector<boxed_array<uint8_t>> usernames;

			usernames.reserve(
				this->clients.size()
			);

			std::size_t anonymous = 0;

			for (auto& client : clients)
				if (auto& name = client.name)
					usernames.emplace_back(*name);
				else
					++anonymous;

			if (anonymous > 0) {
				std::stringstream stream;
				stream << "anonymous(" << anonymous << ")";

				auto string = stream.str();

				usernames.emplace_back(
					string.begin(),
					string.end()
				);
			}

			return usernames;
		}


		int poll() noexcept {
			return ::poll(
				this->poll_sockets.data(),
				this->poll_sockets.size(),
				-1 // infinite timeout
			);
		}


		// Accept new connection.
		void accept() {
			std::cout << "incoming client, ";

			this->clients.emplace_back(
				tp3::socket::connection(this->socket)
			);

			this->poll_sockets.emplace_back(
				pollfd {
					.fd = this->clients.back().descriptor(),
					.events = POLLIN
				}
			);

			std::cout << "accepted." << std::endl;
		}


		// Process one message from the given client.
		void process_client(clients_iter client) {
			if (auto message = client->read())
				std::visit(
					tp3::util::overload {
						[&](const message::name& msg) {
							std::cout << "set name to '" << msg.text << "', ";

							if (this->catalogue.find(msg.text) != this->catalogue.end()) {
								std::cout << "there is already a client with that name, denying."
								          << std::endl;

								client->send(
									tp3::client::message::invalid_name()
								);

								return;
							}

							client->name = std::move(msg.text);

							this->catalogue[*client->name] = this->clients.size() - 1;

							std::cout << "done." << std::endl;
						},

						[&](const message::list_users&) {
							client->send(
								tp3::client::message::users_list(
									this->list_users()
								)
							);
						},

						[&](message::broadcast& msg) {
							auto packet = tp3::client::message::encode(
								tp3::client::message::text(
									boxed_array<uint8_t>(
										client->name ? *client->name
										             : tp3::server::client<buffer_size>::anon_name
									),
									std::move(msg.text)
								)
							);

							// avoid sending message to sender:

							for (auto other = this->clients.begin(); other != client; ++other)
								other->send(packet);

							for (auto other = client + 1; other != this->clients.end(); ++other)
								other->send(packet);
						},

						[&](message::unicast& msg) {
							const auto target = this->catalogue.find(msg.target);

							if (target == this->catalogue.end()) {
								client->send(
									tp3::client::message::invalid_name()
								);

								return;
							}

							const auto& target_client = this->clients[target->second];

							target_client.send(
								tp3::client::message::text(
									boxed_array<uint8_t>(
										client->name ? *client->name
										             : tp3::server::client<buffer_size>::anon_name
									),
									std::move(msg.text)
								)
							);
						}
					},
					*message
				);
		}


		// Run the server's event loop.
		// This function does not return (infinite loop), but it may throw exceptions.
		void process() {
			while (true) {
				if (this->poll() < 0)
					throw std::system_error(errno, std::generic_category());

				// handle server socket:
				auto socket = this->poll_sockets.begin();

				if (socket->revents & POLLIN) { // new client incoming
					this->accept();
					// accept inserts in poll_sockets, invalidating all iterators:
					socket = this->poll_sockets.begin();
				}

				// handle client connections:
				++socket;
				auto end = this->poll_sockets.end();

				while (socket != end) {
					if (socket->revents & POLLIN) { // incoming message
						auto client = this->get_client(socket);

						if (client->connected())
							this->process_client(client);
						else { // client disconnected, remove from collection:
							if (auto& name = client->name)
								this->catalogue.erase(*name);

							tp3::util::algorithm::swap_pop(this->poll_sockets, socket);
							tp3::util::algorithm::swap_pop(this->clients, client);

							if (this->clients.empty())
								// make sure our iterators are not invalid:
								// if the container is unitary, swap_pop will clear it, invalidating all
								// iterators. Therefore, we must break from the loop because socket is no
								// longer valid.
								break;

							// iterator was invalidated by removing from the vector:
							end = this->poll_sockets.end();

							// the last client was swapped, therefore we must update its index in the
							// catalogue.
							if (auto& name = client->name)
								this->catalogue[*name] = client - this->clients.begin();
						}
					}

					++socket;
				}
			}
		}
	};
}
