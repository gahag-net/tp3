#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include <socket/connection.hpp>
#include <socket/server.hpp>

#include <server/message.hpp>
#include <client/message.hpp>
#include <util/read_buffer.hpp>
#include <util/boxed_array.hpp>


namespace tp3::server {
	// A client in the server.
	template<std::size_t buffer_size>
	class client {
		static_assert(buffer_size >= message::min_size);

		template<typename T>
		using boxed_array = tp3::util::boxed_array<T>;


	protected:
		tp3::socket::connection connection;
		tp3::util::read_buffer<buffer_size> read_buffer;


	public:
		static const inline boxed_array<uint8_t> anon_name = boxed_array<uint8_t>("anonymous");

		std::optional<boxed_array<uint8_t>> name; // A client might be anonymous.


		client(tp3::socket::connection&& connection)
			: connection(std::move(connection)) { }

		client(const client&) = delete;
		client(client&&) = default;
		client& operator=(const client&) = delete;
		client& operator=(client&&) = default;


		int descriptor() const noexcept {
			return this->connection.descriptor();
		}


		bool connected() const {
			return !this->connection.is_closed();
		}


		std::optional<message::variant> read() {
			return this->read_buffer.template read<message::variant>(
				this->connection,
				message::decode<typename decltype(read_buffer)::parser_iter>,
				message::token_value(message::token::heading),
				message::token_value(message::token::end)
			);
		}


		void send(tp3::client::message::variant&& message) const {
			this->send(
				tp3::client::message::encode(
					std::move(message)
				)
			);
		}

		void send(const boxed_array<uint8_t>& packet) const {
			this->connection.send(
				packet.get(),
				packet.size()
			);
		}
	};
}
