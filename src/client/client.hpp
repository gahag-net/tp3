#pragma once

#include <poll.h>

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <optional>
#include <string>
#include <system_error>
#include <variant>

#include <client/server.hpp>
#include <socket/addr.hpp>
#include <util/overload.hpp>
#include <util/boxed_array.hpp>


namespace tp3::client {
	template<std::size_t buffer_size>
	class client {
		template<typename T>
		using boxed_array = tp3::util::boxed_array<T>;


	protected:
		tp3::client::server<buffer_size> server;

		std::array<pollfd, 2> poll_files; // stdin, server socket


	public:
		client(tp3::socket::addr&& address)
			: server(std::move(address)),
			  poll_files {
			  	pollfd {
			  		.fd = ::fileno(stdin),
			  		.events = POLLIN
			  	},
			  	pollfd {
			  		.fd = this->server.descriptor(),
			  		.events = POLLIN
			  	}
			  } { }


		client(const client&) = delete;
		client(client&&) = default;
		client& operator=(const client&) = delete;
		client& operator=(client&&) = default;


		int poll() noexcept {
			return ::poll(
				this->poll_files.data(),
				this->poll_files.size(),
				-1 // infinite timeout
			);
		}


		std::optional<tp3::server::message::variant> parse(const std::string& input) {
			if (input == "users")
				return tp3::server::message::list_users();

			const auto begin = input.begin();
			const auto end = input.end();

			const auto delimiter = std::find(begin, end, ';');

			if (delimiter == end)
				return {};

			auto equals = [](auto begin, auto end, const char* str) {
				return std::equal(
					begin,
					end,
					str,
					str + ::strlen(str)
				);
			};

			if (equals(begin, delimiter, "all")) {
				return tp3::server::message::broadcast(
					boxed_array<uint8_t>(delimiter + 1, end)
				);
			}

			if (equals(begin, delimiter, "name")) {
				return tp3::server::message::name(
					boxed_array<uint8_t>(delimiter + 1, end)
				);
			}

			if (equals(begin, delimiter, "uni")) {
				auto delimiter2 = std::find(delimiter + 1, end, ';');

				if (delimiter2 == end)
					return {};

				return tp3::server::message::unicast(
					boxed_array<uint8_t>(delimiter + 1, delimiter2),
					boxed_array<uint8_t>(delimiter2 + 1, end)
				);
			}

			return {};
		}


		void process_incoming_message() {
			if (auto message = this->server.read()) {
				std::visit(
					tp3::util::overload {
						[](const tp3::client::message::error& msg) {
							switch (msg.token) {
								case tp3::client::message::error_token::invalid_name:
									std::cout << "error: invalid name (may be already in use)";
									break;

								case tp3::client::message::error_token::invalid_target:
									std::cout << "error: invalid target";
									break;
							}
						},

						[](const tp3::client::message::users_list& msg) {
							std::cout << "users:" << std::endl;

							for (const auto& user : msg.users)
								std::cout << user << std::endl;
						},

						[](const tp3::client::message::text& msg) {
							std::cout << msg.sender << ": " << msg.body;
						}
					},
					*message
				);

				std::cout << std::endl;
			}
		}


		void process() {
			std::string input;

			while (true) {
				if (this->poll() < 0)
					throw std::system_error(errno, std::generic_category());

				auto& stdin = this->poll_files.front();

				if (stdin.revents & POLLIN) {
					input.clear();

					if (!std::getline(std::cin, input)) {
						if (std::cin.bad())
							std::cerr << "input error!" << std::endl;

						return;
					}

					if (input == "exit")
						return;

					if (auto message = this->parse(input))
						this->server.send(std::move(*message));
					else
						std::cout << "invalid message." << std::endl;
				}

				auto& server = this->poll_files.back();

				if (server.revents & POLLIN)
					this->process_incoming_message();
			}
		}
	};
}
