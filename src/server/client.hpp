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
	template<std::size_t buffer_size>
	class client {
		static_assert(buffer_size >= message::min_size);

	protected:
		tp3::socket::connection connection;
		tp3::util::read_buffer<buffer_size> read_buffer;

		tp3::util::boxed_array<uint8_t> _name;

	public:
		client(tp3::socket::connection&& connection)
			: connection(std::move(connection)),
			  _name(9)
		{
			// TODO: read client name.
			std::string batata = "batatinha";
			std::copy(batata.begin(), batata.end(), this->_name.begin());
		}

		client(const client&) = delete;
		client(client&&) = default;
		client& operator=(const client&) = delete;
		client& operator=(client&&) = default;


		const tp3::util::boxed_array<uint8_t>& name() const {
			return this->_name;
		}

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
			auto packet = tp3::client::message::encode(
				std::move(message)
			);

			this->send(packet);
		}

		void send(const tp3::util::boxed_array<uint8_t>& packet) const {
			this->connection.send(
				packet.get(),
				packet.size()
			);
		}
	};
}
