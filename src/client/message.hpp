#pragma once

#include <algorithm>
#include <numeric>
#include <cstdint>
#include <type_traits>
#include <variant>
#include <optional>
#include <vector>

#include <util/boxed_array.hpp>
#include <util/overload.hpp>


namespace tp3::client::message {
	enum class token : uint8_t {
		users_list = 0x05, // Enquiry character
		text = 0x9E,       // Private message character
		heading = 0x01,    // Start of heading character
		end = 0x04,        // End of transmission character
		user_sep = 0x1F,   // Unit separator character
		text_start = 0x02  // Start of text character
	};

	constexpr auto token_value(token tok) noexcept {
		return static_cast<
			typename std::underlying_type<token>::type
		>(tok);
	}


	template<typename T>
	using boxed_array = tp3::util::boxed_array<T>;


	class users_list {
	public:
		static constexpr std::size_t min_size = 3; // minimum message size.

		std::vector<boxed_array<uint8_t>> users;

		users_list(const users_list&) = delete;
		users_list(users_list&& other) noexcept = default;
		users_list(std::vector<boxed_array<uint8_t>>&& users) noexcept
			: users(std::move(users)) { }

		template<typename ForwardIterator>
		static std::optional<users_list> decode(ForwardIterator& begin, ForwardIterator end) {
			if (std::distance(begin, end) < users_list::min_size)
				return {};

			if (*begin != token_value(token::heading))
				return {};

			++begin;

			if (*begin != token_value(token::users_list))
				return {};

			++begin;

			ForwardIterator separator;

			auto find_separator = [&] {
				separator = std::find(begin, end, token::user_sep);

				if (separator == end)
					separator = std::find(begin, end, token::end);

				return separator == end;
			};

			std::vector<boxed_array<uint8_t>> users;

			while (find_separator()) {
				users.emplace_back(begin, separator);

				begin = separator + 1;
			}

			if (*begin != token_value(token::end))
				return {};

			++begin; // leave begin at the end of the parsed data.

			return users_list(
				std::move(users)
			);
		}
	};


	class text {
	public:
		static constexpr std::size_t min_size = 4; // minimum message size.

		boxed_array<uint8_t> sender;
		boxed_array<uint8_t> msg;

		text(const text&) = delete;
		text(text&& other) noexcept = default;
		text(boxed_array<uint8_t>&& sender, boxed_array<uint8_t>&& msg) noexcept
			: sender(std::move(sender)),
			  msg(std::move(msg)) { }

		template<typename ForwardIterator>
		static std::optional<text> decode(ForwardIterator& begin, ForwardIterator end) {
			if (std::distance(begin, end) < text::min_size)
				return {};

			if (*begin != token_value(token::heading))
				return {};

			++begin;

			if (*begin != token_value(token::text))
				return {};

			const auto sender = begin + 1;

			const auto sender_end = std::find(
				sender,
				end,
				token_value(token::text_start)
			);

			if (sender_end == end)
				return {};

			const auto msg = sender_end + 1;

			const auto msg_end = std::find(
				msg,
				end,
				token_value(token::end)
			);

			if (msg_end == end)
				return {};

			begin = msg_end + 1;  // leave begin at the end of the parsed data.

			return text(
				boxed_array<uint8_t>(sender, sender_end),
				boxed_array<uint8_t>(msg, msg_end)
			);
		}
	};


	static constexpr std::size_t min_size = [] { // minimum message size.
		const auto messages = {
			users_list::min_size,
			text::min_size
		};

		return *std::max_element(
			messages.begin(),
			messages.end()
		);
	}();

	using variant = std::variant<
		users_list,
		text
	>;


	template<typename ForwardIterator>
	std::optional<variant> decode(ForwardIterator& begin, ForwardIterator end) {
		const ForwardIterator _begin = begin;

		if (const auto message = users_list::decode(begin, end))
			return message;

		begin = _begin; // rollback

		if (const auto message = text::decode(begin, end))
			return message;

		return {};
	}


	boxed_array<uint8_t> encode(variant&& message) {
		return std::visit(
			tp3::util::overload {
				[](const users_list& msg) -> boxed_array<uint8_t> {
					const std::size_t size = std::accumulate(
						msg.users.begin(),
						msg.users.end(),
						2, // heading + users_list
						[](const auto& acc, const auto& user) {
							return acc + user.size() + 1;
						}
					);

					boxed_array<uint8_t> packet(size);

					auto packet_it = packet.begin();

					*packet_it++ = token_value(token::heading);
					*packet_it++ = token_value(token::users_list);

					for (const auto& user : msg.users) {
						packet_it = std::copy(
							user.begin(),
							user.end(),
							packet_it
						);

						*packet_it++ = token_value(token::user_sep);
					}

					*packet_it++ = token_value(token::end);

					return packet;
				},

				[](const text& msg) -> boxed_array<uint8_t> {
					const std::size_t size = 4 // heading + text + text_start + end
					                       + msg.sender.size()
					                       + msg.msg.size();

					boxed_array<uint8_t> packet(size);

					auto packet_it = packet.begin();

					*packet_it++ = token_value(token::heading);
					*packet_it++ = token_value(token::text);

					packet_it = std::copy(
						msg.sender.begin(),
						msg.sender.end(),
						packet_it
					);

					*packet_it++ = token_value(token::text_start);

					packet_it = std::copy(
						msg.msg.begin(),
						msg.msg.end(),
						packet_it
					);

					*packet_it++ = token_value(token::end);

					return packet;
				}
			},
			message
		);
	}
}
