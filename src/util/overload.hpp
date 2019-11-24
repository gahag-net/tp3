#pragma once


namespace tp3::util {
	template<typename... Ts> struct overload : Ts... {
		using Ts::operator()...;
	};

	template<typename... Ts> overload(Ts...) -> overload<Ts...>;
}
