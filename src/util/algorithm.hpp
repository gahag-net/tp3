#pragma once

#include <algorithm>


namespace tp3::util::algorithm {
	template<typename Container>
	void swap_pop(Container& container, typename Container::iterator it) {
		if (container.size() < 2) {
			container.clear();
			return;
		}

		std::iter_swap(it, container.end() - 1);
		container.pop_back();
	}
}
