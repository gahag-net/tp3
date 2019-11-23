#pragma once


namespace tp3::util {
	template<typename Token>
	constexpr auto token_value(Token tok) noexcept {
		return static_cast<
			typename std::underlying_type<Token>::type
		>(tok);
	}
}
