#pragma once

#include <algorithm>


namespace tp3::util::algorithm {
	// Swap and pop algorithm.
	// If container.size() > 1, container.end() and container.end() - 1 are invalidated.
	// Otherwise, all iterators are invalidated.
	template<typename Container>
	void swap_pop(Container& container, typename Container::iterator it) {
		if (container.size() < 2) {
			container.clear();
			return;
		}

		auto last = container.end() - 1;

		if (it != last)
			std::iter_swap(it, last);

		container.pop_back();
	}
}
