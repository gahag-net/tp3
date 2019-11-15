#pragma once


namespace tp3::util {
	template<class... Ts> struct overload : Ts... {
		using Ts::operator()...;
	};

	template<class... Ts> overload(Ts...) -> overload<Ts...>;
}
