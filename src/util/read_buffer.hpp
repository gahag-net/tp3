#pragma once

#include <iostream>

#include <algorithm>
#include <cstdint>
#include <vector>
#include <optional>

#include <socket/connection.hpp>


namespace tp3::util {
	template<std::size_t size>
	class read_buffer {
	protected:
		std::vector<uint8_t> buffer;


		void read(const tp3::socket::connection& connection) {
			const auto current_size = this->buffer.size();

			if (current_size == size) // buffer is full.
				return;

			this->buffer.resize(size);

			uint8_t* end = &this->buffer[current_size];

			const auto added_size = connection.recv(
				end,
				size - current_size
			);

			this->buffer.resize(current_size + added_size);
		}


	public:
		using parser_iter = decltype(buffer)::iterator;


		read_buffer(const read_buffer&) = delete;
		read_buffer(read_buffer&&) noexcept = default;
		read_buffer& operator=(const read_buffer&) = delete;
		read_buffer& operator=(read_buffer&&) = default;

		read_buffer() noexcept {
			this->buffer.reserve(size);
		}


		template<typename Message, typename Token, typename Parser>
		std::optional<Message> read(
			const tp3::socket::connection& connection,
			Parser parser,
			Token heading_tok,
			Token end_tok
		) {
			this->read(connection);

			auto begin = this->buffer.begin();
			const auto end = this->buffer.end();

			std::optional<Message> message;

			while (!message) {
				begin = std::find(begin, end, heading_tok);

				if (begin == end) {
					this->buffer.clear();
					return {};
				}

				auto msg_end = std::find(begin, end, end_tok);

				if (msg_end == end) {
					if (this->buffer.size() == size)
						// buffer is full with a single message.
						this->buffer.clear();

					return {};
				}

				auto m2 = parser(
					begin,
					msg_end + 1
				);

				message = std::move(m2);
			}

			this->buffer.erase(
				this->buffer.begin(),
				begin
			);

			return message;
		}
	};
}
