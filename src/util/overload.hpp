#pragma once


namespace tp3::util {
	// Utility for std::visit.
	template<typename... Ts> struct overload : Ts... {
		using Ts::operator()...;
	};

	template<typename... Ts> overload(Ts...) -> overload<Ts...>;
}
