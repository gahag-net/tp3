#pragma once

#include <algorithm>
#include <iterator>
#include <cstddef>
#include <memory>


namespace tp3::util {
	template<typename T>
	class boxed_array {
	protected:
		std::unique_ptr<T[]> data;
		std::size_t _size; // this should only be changed when moving.

	public:
		boxed_array() noexcept
			: _size(0),
			  data() { }

		boxed_array(std::size_t size)
			: _size(size),
			  data(
			  	std::make_unique<T[]>(size)
			  ) { }

		template<typename ForwardIterator>
		boxed_array(ForwardIterator begin, ForwardIterator end)
			: boxed_array(
			  	std::distance(begin, end)
			  )
		{
			std::copy(
				begin,
				end,
				this->data.get()
			);
		}

		boxed_array(const boxed_array&) = delete;
		boxed_array(boxed_array&& other) noexcept = default;
		boxed_array<T>& operator=(const boxed_array<T>&) = delete;
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
	};
}
