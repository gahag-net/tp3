#pragma once

#include <cstddef>

#include <socket/connection.hpp>
#include <client/message.hpp>
#include <server/message.hpp>
#include <util/read_buffer.hpp>
#include <util/token.hpp>


namespace tp3::client {
	template<std::size_t buffer_size>
	class server {
		static_assert(buffer_size >= message::min_size);


	protected:
		tp3::socket::connection connection;
		tp3::util::read_buffer<buffer_size> read_buffer;


	public:
		server(tp3::socket::addr&& addr)
			: connection(std::move(addr)) { }

		server(const server&) = delete;
		server(server&&) = default;
		server& operator=(const server&) = delete;
		server& operator=(server&&) = default;


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
				util::token_value(message::token::heading),
				util::token_value(message::token::end)
			);
		}


		void send(tp3::server::message::variant&& message) const {
			auto packet = tp3::server::message::encode(
				std::move(message)
			);

			this->connection.send(
				packet.get(),
				packet.size()
			);
		}
	};
}
