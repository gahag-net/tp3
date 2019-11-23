#pragma once

#include <algorithm>
#include <cstdint>
#include <type_traits>
#include <variant>
#include <optional>

#include <util/boxed_array.hpp>
#include <util/overload.hpp>


// Messages received by the server.
namespace tp3::server::message {
	enum class token : uint8_t {
		name = 0x84,			 // Index character.
		list_users = 0x05, // Enquiry character.
		broadcast = 0x02,  // Start of text character.
		unicast = 0x9E,    // Private message character.
		heading = 0x01,    // Start of heading character.
		end = 0x04,        // End of transmission character.
		text = 0x02        // Start of text character.
	};

	constexpr auto token_value(token tok) noexcept {
		return static_cast<
			typename std::underlying_type<token>::type
		>(tok);
	}


	template<typename T>
	using boxed_array = tp3::util::boxed_array<T>;


	// Set name message.
	class name {
	public:
		static constexpr std::size_t min_size = 3; // minimum message size.

		boxed_array<uint8_t> text;


		name(const name&) = delete;
		name(name&& other) noexcept = default;
		name(boxed_array<uint8_t>&& text)
			: text(std::move(text)) { }

		name& operator=(const name&) = delete;
		name& operator=(name&&) = default;


		template<typename ForwardIterator>
		static std::optional<name> decode(ForwardIterator& begin, ForwardIterator end) {
			if (std::distance(begin, end) < name::min_size)
				return {};

			if (*begin != token_value(token::heading))
				return {};

			++begin;

			if (*begin != token_value(token::name))
				return {};

			const auto text = begin + 1;

			const auto text_end = std::find(
				text,
				end,
				token_value(token::end)
			);

			if (text_end == end)
				return {};

			begin = text_end + 1;  // leave begin at the end of the parsed data.

			return name(
				boxed_array<uint8_t>(text, text_end)
			);
		}
	};


	class list_users {
	public:
		static constexpr std::size_t min_size = 3; // minimum message size.

		list_users(const list_users&) = delete;
		list_users(list_users&& other) noexcept = default;
		list_users() noexcept = default;

		list_users& operator=(const list_users&) = delete;
		list_users& operator=(list_users&&) = default;


		template<typename ForwardIterator>
		static std::optional<list_users> decode(ForwardIterator& begin, ForwardIterator end) {
			if (std::distance(begin, end) < list_users::min_size)
				return {};

			if (*begin != token_value(token::heading))
				return {};

			++begin;

			if (*begin != token_value(token::list_users))
				return {};

			++begin;

			if (*begin != token_value(token::end))
				return {};

			++begin; // leave begin at the end of the parsed data.

			return list_users();
		}
	};


	class broadcast {
	public:
		static constexpr std::size_t min_size = 3; // minimum message size.

		boxed_array<uint8_t> text;


		broadcast(const broadcast&) = delete;
		broadcast(broadcast&& other) noexcept = default;
		broadcast(boxed_array<uint8_t>&& text)
			: text(std::move(text)) { }

		broadcast& operator=(const broadcast&) = delete;
		broadcast& operator=(broadcast&&) = default;


		template<typename ForwardIterator>
		static std::optional<broadcast> decode(ForwardIterator& begin, ForwardIterator end) {
			if (std::distance(begin, end) < broadcast::min_size)
				return {};

			if (*begin != token_value(token::heading))
				return {};

			++begin;

			if (*begin != token_value(token::broadcast))
				return {};

			const auto text = begin + 1;

			const auto text_end = std::find(
				text,
				end,
				token_value(token::end)
			);

			if (text_end == end)
				return {};

			begin = text_end + 1;  // leave begin at the end of the parsed data.

			return broadcast(
				boxed_array<uint8_t>(text, text_end)
			);
		}
	};


	class unicast {
	public:
		static constexpr std::size_t min_size = 4; // minimum message size.

		boxed_array<uint8_t> target;
		boxed_array<uint8_t> text;

		unicast(const unicast&) = delete;
		unicast(unicast&& other) noexcept = default;
		unicast(boxed_array<uint8_t>&& target, boxed_array<uint8_t>&& text)
			: target(std::move(target)),
			  text(std::move(text)) { }

		unicast& operator=(const unicast&) = delete;
		unicast& operator=(unicast&&) = default;


		template<typename ForwardIterator>
		static std::optional<unicast> decode(ForwardIterator& begin, ForwardIterator end) {
			if (std::distance(begin, end) < unicast::min_size)
				return {};

			if (*begin != token_value(token::heading))
				return {};

			++begin;

			if (*begin != token_value(token::unicast))
				return {};

			const auto target = begin + 1;

			const auto target_end = std::find(
				target,
				end,
				token_value(token::text)
			);

			if (target_end == end)
				return {};

			const auto text = target_end + 1;

			const auto text_end = std::find(
				text,
				end,
				token_value(token::end)
			);

			if (text_end == end)
				return {};

			begin = text_end + 1;  // leave begin at the end of the parsed data.

			return unicast(
				boxed_array<uint8_t>(target, target_end),
				boxed_array<uint8_t>(text, text_end)
			);
		}
	};


	static constexpr std::size_t min_size = [] { // minimum message size.
		const auto messages = {
			name::min_size,
			list_users::min_size,
			broadcast::min_size,
			unicast::min_size
		};

		return *std::max_element(
			messages.begin(),
			messages.end()
		);
	}();

	using variant = std::variant<
		name,
		list_users,
		broadcast,
		unicast
	>;


	template<typename ForwardIterator>
	std::optional<variant> decode(ForwardIterator& begin, ForwardIterator end) {
		const ForwardIterator _begin = begin;

		if (auto message = name::decode(begin, end))
			return std::move(*message);

		begin = _begin; // rollback

		if (auto message = list_users::decode(begin, end))
			return std::move(*message);

		begin = _begin; // rollback

		if (auto message = broadcast::decode(begin, end))
			return std::move(*message);

		begin = _begin; // rollback

		if (auto message = unicast::decode(begin, end))
			return std::move(*message);

		return {};
	}


	boxed_array<uint8_t> encode(variant&& message) {
		return std::visit(
			tp3::util::overload {
				[](const name& msg) -> boxed_array<uint8_t> {
					const std::size_t size = 3 // heading + name + end
					                       + msg.text.size();

					boxed_array<uint8_t> packet(size);

					auto packet_it = packet.begin();

					*packet_it++ = token_value(token::heading);
					*packet_it++ = token_value(token::name);

					packet_it = std::copy(
						msg.text.begin(),
						msg.text.end(),
						packet_it
					);

					*packet_it++ = token_value(token::end);

					return packet;
				},

				[](const list_users& msg) -> boxed_array<uint8_t> {
					const std::size_t size = 3; // heading + list_users + end

					boxed_array<uint8_t> packet(size);

					auto packet_it = packet.begin();

					*packet_it++ = token_value(token::heading);
					*packet_it++ = token_value(token::list_users);
					*packet_it++ = token_value(token::end);

					return packet;
				},

				[](const broadcast& msg) -> boxed_array<uint8_t> {
					const std::size_t size = 3 // heading + broadcast + end
					                       + msg.text.size();

					boxed_array<uint8_t> packet(size);

					auto packet_it = packet.begin();

					*packet_it++ = token_value(token::heading);
					*packet_it++ = token_value(token::broadcast);

					packet_it = std::copy(
						msg.text.begin(),
						msg.text.end(),
						packet_it
					);

					*packet_it++ = token_value(token::end);

					return packet;
				},

				[](const unicast& msg) -> boxed_array<uint8_t> {
					const std::size_t size = 4 // heading + unicast + text + end
					                       + msg.target.size()
					                       + msg.text.size();

					boxed_array<uint8_t> packet(size);

					auto packet_it = packet.begin();

					*packet_it++ = token_value(token::heading);
					*packet_it++ = token_value(token::unicast);

					packet_it = std::copy(
						msg.target.begin(),
						msg.target.end(),
						packet_it
					);

					*packet_it++ = token_value(token::text);

					packet_it = std::copy(
						msg.text.begin(),
						msg.text.end(),
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
