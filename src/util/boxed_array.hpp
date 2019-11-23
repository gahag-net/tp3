#pragma once

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <iterator>
#include <memory>
#include <type_traits>


namespace tp3::util {
	// A dynamic, fixed sized array.
	template<typename T>
	class boxed_array {
	protected:
		std::unique_ptr<T[]> data;
		std::size_t _size; // The size is fixed, should only be changed when moving.

	public:
		// Construct empty array.
		boxed_array() noexcept
			: _size(0),
			  data() { }

		// Construct `size` default initialized elements.
		boxed_array(std::size_t size)
			: _size(size),
			  data(
			  	std::make_unique<T[]>(size)
			  ) { }

		// Construct from an iterator.
		template<typename ForwardIterator>
		boxed_array(ForwardIterator begin, ForwardIterator end)
			: boxed_array(
			  	std::distance(begin, end)
			  )
		{
			std::copy(
				begin,
				end,
				this->get()
			);
		}

		// Construct from a static string, only enabled when T ~ char.
		template<
			typename Char = std::enable_if_t<
				std::is_same<
					std::make_unsigned_t<T>,
					unsigned char
				>::value,
				char
			>
		>
		boxed_array(const Char* string)
			: boxed_array(
			  	::strlen(string)
			  )
		{
			std::copy(
				string,
				string + this->_size,
				this->get()
			);
		}

		// Copy constructor.
		boxed_array(const boxed_array& other) {
			this->_size = other._size;
			this->data = std::make_unique<T[]>(other._size);

			std::copy(
				other.begin(),
				other.end(),
				this->get()
			);
		}
		// Move constructor.
		boxed_array(boxed_array&& other) noexcept = default;
		// Copy assignment.
		boxed_array<T>& operator=(const boxed_array<T>& other) {
			*this = boxed_array(other);
			return *this;
		};
		// Move assignment.
		boxed_array<T>& operator=(boxed_array<T>&&) = default;


		T* get() noexcept {
			return this->data.get();
		}

		const T* get() const noexcept {
			return this->data.get();
		}

		T& operator*() {
			return *(this->data);
		}

		const T& operator*() const {
			return *(this->data);
		}

		T* operator->() noexcept {
			return this->get();
		}

		const T* operator->() const noexcept {
			return this->get();
		}

		T& operator[](std::size_t ix) {
			return this->data[ix];
		}

		const T& operator[](std::size_t ix) const {
			return this->data[ix];
		}

		std::size_t size() const noexcept {
			return this->_size;
		}

		void swap(boxed_array<T>& other) noexcept {
			std::swap(this->data, other.data);
			std::swap(this->_size, other._size);
		}

		T* begin() noexcept {
			return this->get();
		}

		const T* begin() const noexcept {
			return this->get();
		}

		const T* end() const noexcept {
			return this->begin() + this->_size;
		}

		T* end() noexcept {
			return this->begin() + this->_size;
		}


		bool operator==(const boxed_array& other) const {
			return this->size() == other.size()
			    && std::equal(
			       	this->begin(),
			       	this->end(),
			       	other.begin()
			       );
		}
	};
}

template<typename T>
struct std::hash<tp3::util::boxed_array<T>> {
	std::size_t operator()(const tp3::util::boxed_array<T>& box) const noexcept {
		std::size_t seed = box.size();

		for(const auto& e : box)
			seed ^= std::hash<T>{}(e)
			      + 0x9e3779b9
			      + (seed << 6)
			      + (seed >> 2);

		return seed;
	}
};

template<typename T>
std::ostream& operator<<(std::ostream &o, const tp3::util::boxed_array<T>& array) {
	for (auto b : array)
		o << b;

	return o;
}
