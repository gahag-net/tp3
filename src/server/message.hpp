#pragma once

#include <algorithm>
#include <cstdint>
#include <type_traits>
#include <variant>
#include <optional>

#include <util/boxed_array.hpp>


namespace tp3::server::message {
	enum class token : uint8_t {
		list_users = 0x05, // Enquiry character
		broadcast = 0x02,  // Start of text character
		unicast = 0x9E,    // Private message character
		heading = 0x01,    // Start of heading character
		end = 0x04,        // End of transmission character
		text = 0x02        // Start of text character
	};

	constexpr auto token_value(token tok) noexcept {
    return static_cast<
			typename std::underlying_type<token>::type
		>(tok);
	}


	template<typename T>
	using boxed_array = tp3::util::boxed_array<T>;


	class list_users {
	public:
		list_users(const list_users&) = delete;
		list_users(list_users&& other) noexcept = default;
		list_users() noexcept = default;

		template<typename ForwardIterator>
		static std::optional<list_users> from(ForwardIterator& begin, ForwardIterator end) {
			if (std::distance(begin, end) < 3) // minimum message size is 3.
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
		boxed_array<uint8_t> text;


		broadcast(const broadcast&) = delete;
		broadcast(broadcast&& other) noexcept = default;
		broadcast(boxed_array<uint8_t>&& text)
			: text(std::move(text)) { }

		template<typename ForwardIterator>
		static std::optional<broadcast> from(ForwardIterator& begin, ForwardIterator end) {
			if (std::distance(begin, end) < 3) //  items will be read below 3.
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
		boxed_array<uint8_t> target;
		boxed_array<uint8_t> text;

		unicast(const unicast&) = delete;
		unicast(unicast&& other) noexcept = default;
		unicast(boxed_array<uint8_t>&& target, boxed_array<uint8_t>&& text)
			: target(std::move(target)),
			  text(std::move(text)) { }

		template<typename ForwardIterator>
		static std::optional<unicast> from(ForwardIterator& begin, ForwardIterator end) {
			if (std::distance(begin, end) < 4) // minimum message size is 4.
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


	using variant = std::variant<
		list_users,
		broadcast,
		unicast
	>;

	template<typename ForwardIterator>
	std::optional<variant> parse(ForwardIterator& begin, ForwardIterator end) {
		ForwardIterator _begin = begin;

		if (auto message = list_users::from(begin, end))
			return message;

		begin = _begin; // rollback

		if (auto message = broadcast::from(begin, end))
			return message;

		begin = _begin; // rollback

		if (auto message = unicast::from(begin, end))
			return message;

		return {};
	}
}
