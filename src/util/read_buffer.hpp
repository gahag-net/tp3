#pragma once

#include <iostream>

#include <algorithm>
#include <cstdint>
#include <vector>
#include <optional>

#include <socket/connection.hpp>


namespace tp3::util {
	// A message read buffer for a connection socket.
	template<std::size_t size>
	class read_buffer {
	protected:
		// The inner buffer to read.
		// This buffer *must not* change capacity. The capacity supposed to be fixed, and only
		// the size should variate between 0 and capacity.
		std::vector<uint8_t> buffer;


		// Read bytes into buffer.
		void read(const tp3::socket::connection& connection) {
			const auto current_size = this->buffer.size();

			if (current_size == size) // Buffer is full.
				return;

			this->buffer.resize(size); // Extend the buffer to the end for reading.

			uint8_t* end = &this->buffer[current_size];

			const auto added_size = connection.recv(
				end,
				size - current_size
			);

			this->buffer.resize(current_size + added_size); // Remove excess elements.
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


		// Read a message from the buffer.
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

				if (begin == end) { // heading token not found, data must be trash.
					this->buffer.clear();
					return {};
				}

				auto msg_end = std::find(begin, end, end_tok);

				if (msg_end == end) { // end token not found
					if (this->buffer.size() == size)
						// the message is larger than the buffer, so we can't handle it.
						this->buffer.clear();

					return {};
				}

				// The parser should move begin to the point where it consumed.
				message = parser(
					begin,
					msg_end + 1
				);
			}

			// A message has been parsed, remove it from the buffer.
			this->buffer.erase(
				this->buffer.begin(),
				begin
			);

			return message;
		}
	};
}
