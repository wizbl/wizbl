
#ifndef _WIZBLCOIN_PRE_VECTOR_H_
#define _WIZBLCOIN_PRE_VECTOR_H_

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <iterator>
#include <type_traits>

#pragma pack(push, 1)
template<unsigned int N, typename T, typename Size = uint32_t, typename Diff = int32_t>
class prevector {
public:
	typedef T type_of_value;
	typedef const type_of_value& const_refer;
	typedef type_of_value& refer;
	typedef const type_of_value* const_ptr;
	typedef type_of_value* ptr;
	typedef Diff type_of_difference;
	typedef Size type_of_size;

public:
	class iterator {
	public:
		typedef T& refer;
		typedef std::random_access_iterator_tag iterator_category;
		typedef Diff type_of_difference;
		typedef T* ptr;
		typedef T type_of_value;
	public:
		iterator(T* ptr_) : ptr(ptr_) {}
		T* operator->() const { return ptr; }
		T& operator[](type_of_size pos) { return ptr[pos]; }
		T& operator*() const { return *ptr; }
		iterator& operator++() { ptr++; return *this; }
		const T& operator[](type_of_size pos) const { return ptr[pos]; }
		iterator operator++(int) { iterator copy(*this); ++(*this); return copy; }
		iterator& operator--() { ptr--; return *this; }
		type_of_difference friend operator-(iterator a, iterator b) { return (&(*a) - &(*b)); }
		iterator operator--(int) { iterator copy(*this); --(*this); return copy; }
		iterator& operator+=(type_of_size n) { ptr += n; return *this; }
		iterator operator+(type_of_size n) { return iterator(ptr + n); }
		iterator& operator-=(type_of_size n) { ptr -= n; return *this; }
		iterator operator-(type_of_size n) { return iterator(ptr - n); }
		bool operator!=(iterator x) const { return ptr != x.ptr; }
		bool operator==(iterator x) const { return ptr == x.ptr; }
		bool operator<=(iterator x) const { return ptr <= x.ptr; }
		bool operator>=(iterator x) const { return ptr >= x.ptr; }
		bool operator<(iterator x) const { return ptr < x.ptr; }
		bool operator>(iterator x) const { return ptr > x.ptr; }
	private:
		T * ptr;
	};

	class reverse_iterator {
	public:
		typedef T* ptr;
		typedef T type_of_value;
		typedef std::bidirectional_iterator_tag iterator_category;
		typedef Diff type_of_difference;
		typedef T& refer;
	public:
		reverse_iterator(T* ptr_) : ptr(ptr_) {}
		const T& operator*() const { return *ptr; }
		T& operator*() { return *ptr; }
		const T* operator->() const { return ptr; }
		T* operator->() { return ptr; }
		reverse_iterator& operator++() { ptr--; return *this; }
		reverse_iterator& operator--() { ptr++; return *this; }
		reverse_iterator operator--(int) { reverse_iterator copy(*this); --(*this); return copy; }
		reverse_iterator operator++(int) { reverse_iterator copy(*this); ++(*this); return copy; }
		bool operator!=(reverse_iterator x) const { return ptr != x.ptr; }
		bool operator==(reverse_iterator x) const { return ptr == x.ptr; }

	private:
		T * ptr;
	};

	class const_iterator {
	public:
		typedef std::random_access_iterator_tag iterator_category;
		typedef Diff type_of_difference;
		typedef const T& refer;
		typedef const T* ptr;
		typedef const T type_of_value;
	public:
		const_iterator(iterator x) : ptr(&(*x)) {}
		const_iterator(const T* ptr_) : ptr(ptr_) {}
		const T* operator->() const { return ptr; }
		const T& operator*() const { return *ptr; }
		const_iterator& operator++() { ptr++; return *this; }
		const T& operator[](type_of_size pos) const { return ptr[pos]; }
		const_iterator operator++(int) { const_iterator copy(*this); ++(*this); return copy; }
		const_iterator& operator--() { ptr--; return *this; }
		type_of_difference friend operator-(const_iterator a, const_iterator b) { return (&(*a) - &(*b)); }
		const_iterator operator--(int) { const_iterator copy(*this); --(*this); return copy; }
		const_iterator& operator+=(type_of_size n) { ptr += n; return *this; }
		const_iterator operator+(type_of_size n) { return const_iterator(ptr + n); }
		const_iterator& operator-=(type_of_size n) { ptr -= n; return *this; }
		const_iterator operator-(type_of_size n) { return const_iterator(ptr - n); }
		bool operator!=(const_iterator x) const { return ptr != x.ptr; }
		bool operator==(const_iterator x) const { return ptr == x.ptr; }
		bool operator<=(const_iterator x) const { return ptr <= x.ptr; }
		bool operator>=(const_iterator x) const { return ptr >= x.ptr; }
		bool operator<(const_iterator x) const { return ptr < x.ptr; }
		bool operator>(const_iterator x) const { return ptr > x.ptr; }

	private:
		const T* ptr;
	};

	class const_reverse_iterator {
	public:
		typedef std::bidirectional_iterator_tag iterator_category;
		typedef const T& refer;
		typedef Diff type_of_difference;
		typedef const T* ptr;
		typedef const T type_of_value;
	public:
		const_reverse_iterator(reverse_iterator x) : ptr(&(*x)) {}
		const_reverse_iterator(const T* ptr_) : ptr(ptr_) {}
		const T* operator->() const { return ptr; }
		const T& operator*() const { return *ptr; }
		const_reverse_iterator& operator++() { ptr--; return *this; }
		const_reverse_iterator& operator--() { ptr++; return *this; }
		const_reverse_iterator operator--(int) { const_reverse_iterator copy(*this); --(*this); return copy; }
		const_reverse_iterator operator++(int) { const_reverse_iterator copy(*this); ++(*this); return copy; }
		bool operator!=(const_reverse_iterator x) const { return ptr != x.ptr; }
		bool operator==(const_reverse_iterator x) const { return ptr == x.ptr; }

	private:
		const T* ptr;
	};

public:
	template<typename InputIterator>
	void assign(InputIterator first, InputIterator last) {
		type_of_size n = last - first;
		clear();
		if (capacity() < n) {
			change_capacity(n);
		}
		while (first != last) {
			_size++;
			new(static_cast<void*>(item_ptr(size() - 1))) T(*first);
			++first;
		}
	}
	void assign(type_of_size n, const T& val) {
		clear();
		if (capacity() < n) {
			change_capacity(n);
		}
		while (size() < n) {
			_size++;
			new(static_cast<void*>(item_ptr(size() - 1))) T(val);
		}
	}

	prevector() : _size(0), _union{ {} } {}

	explicit prevector(type_of_size n) : _size(0) {
		resize(n);
	}

	explicit prevector(type_of_size n, const T& val = T()) : _size(0) {
		change_capacity(n);
		while (size() < n) {
			_size++;
			new(static_cast<void*>(item_ptr(size() - 1))) T(val);
		}
	}

	template<typename InputIterator>
	prevector(InputIterator first, InputIterator last) : _size(0) {
		type_of_size n = last - first;
		change_capacity(n);
		while (first != last) {
			_size++;
			new(static_cast<void*>(item_ptr(size() - 1))) T(*first);
			++first;
		}
	}

	prevector(const prevector<N, T, Size, Diff>& other) : _size(0) {
		change_capacity(other.size());
		const_iterator it = other.begin();
		while (it != other.end()) {
			_size++;
			new(static_cast<void*>(item_ptr(size() - 1))) T(*it);
			++it;
		}
	}

	prevector(prevector<N, T, Size, Diff>&& other) : _size(0) {
		swap(other);
	}

	~prevector() {
		if (!std::is_trivially_destructible<T>::value) {
			clear();
		}
		if (!is_direct()) {
			free(_union.indirect);
			_union.indirect = nullptr;
		}
	}

	prevector& operator=(const prevector<N, T, Size, Diff>& other) {
		if (&other == this) {
			return *this;
		}
		resize(0);
		change_capacity(other.size());
		const_iterator it = other.begin();
		while (it != other.end()) {
			_size++;
			new(static_cast<void*>(item_ptr(size() - 1))) T(*it);
			++it;
		}
		return *this;
	}
	prevector& operator=(prevector<N, T, Size, Diff>&& other) {
		swap(other);
		return *this;
	}

	const_reverse_iterator rbegin() const { return const_reverse_iterator(item_ptr(size() - 1)); }
	reverse_iterator rbegin() { return reverse_iterator(item_ptr(size() - 1)); }
	const_reverse_iterator rend() const { return const_reverse_iterator(item_ptr(-1)); }
	reverse_iterator rend() { return reverse_iterator(item_ptr(-1)); }

	const_iterator begin() const { return const_iterator(item_ptr(0)); }
	iterator begin() { return iterator(item_ptr(0)); }
	const_iterator end() const { return const_iterator(item_ptr(size())); }
	iterator end() { return iterator(item_ptr(size())); }

	size_t capacity() const {
		if (is_direct()) {
			return N;
		}
		else {
			return _union.capacity;
		}
	}

	bool empty() const {
		return size() == 0;
	}
	type_of_size size() const {
		return is_direct() ? _size : _size - N - 1;
	}

	const T& operator[](type_of_size pos) const {
		return *item_ptr(pos);
	}
	T& operator[](type_of_size pos) {
		return *item_ptr(pos);
	}

	iterator erase(iterator first, iterator last) {
		iterator p = first;
		char* endp = (char*)&(*end());
		if (!std::is_trivially_destructible<T>::value) {
			while (p != last) {
				(*p).~T();
				_size--;
				++p;
			}
		}
		else {
			_size -= last - p;
		}
		memmove(&(*first), &(*last), endp - ((char*)(&(*last))));
		return first;
	}
	iterator erase(iterator pos) {
		return erase(pos, pos + 1);
	}

	void reserve(type_of_size new_capacity) {
		if (new_capacity > capacity()) {
			change_capacity(new_capacity);
		}
	}
	void resize(type_of_size new_size) {
		if (size() > new_size) {
			erase(item_ptr(new_size), end());
		}
		if (new_size > capacity()) {
			change_capacity(new_size);
		}
		while (size() < new_size) {
			_size++;
			new(static_cast<void*>(item_ptr(size() - 1))) T();
		}
	}

	template<typename InputIterator>
	void insert(iterator pos, InputIterator first, InputIterator last) {
		type_of_size p = pos - begin();
		type_of_difference count = last - first;
		type_of_size new_size = size() + count;
		if (capacity() < new_size) {
			change_capacity(new_size + (new_size >> 1));
		}
		memmove(item_ptr(p + count), item_ptr(p), (size() - p) * sizeof(T));
		_size += count;
		while (first != last) {
			new(static_cast<void*>(item_ptr(p))) T(*first);
			++p;
			++first;
		}
	}
	void insert(iterator pos, type_of_size count, const T& value) {
		type_of_size p = pos - begin();
		type_of_size new_size = size() + count;
		if (capacity() < new_size) {
			change_capacity(new_size + (new_size >> 1));
		}
		memmove(item_ptr(p + count), item_ptr(p), (size() - p) * sizeof(T));
		_size += count;
		for (type_of_size i = 0; i < count; i++) {
			new(static_cast<void*>(item_ptr(p + i))) T(value);
		}
	}
	iterator insert(iterator pos, const T& value) {
		type_of_size p = pos - begin();
		type_of_size new_size = size() + 1;
		if (capacity() < new_size) {
			change_capacity(new_size + (new_size >> 1));
		}
		memmove(item_ptr(p + 1), item_ptr(p), (size() - p) * sizeof(T));
		_size++;
		new(static_cast<void*>(item_ptr(p))) T(value);
		return iterator(item_ptr(p));
	}
	void clear() {
		resize(0);
	}
	void shrink_to_fit() {
		change_capacity(size());
	}

	const T& front() const {
		return *item_ptr(0);
	}
	T& front() {
		return *item_ptr(0);
	}

	const T& back() const {
		return *item_ptr(size() - 1);
	}
	T& back() {
		return *item_ptr(size() - 1);
	}

	void swap(prevector<N, T, Size, Diff>& other) {
		std::swap(_union, other._union);
		std::swap(_size, other._size);
	}

	void pop_back() {
		erase(end() - 1, end());
	}
	void push_back(const T& value) {
		type_of_size new_size = size() + 1;
		if (capacity() < new_size) {
			change_capacity(new_size + (new_size >> 1));
		}
		new(item_ptr(size())) T(value);
		_size++;
	}

	bool operator<(const prevector<N, T, Size, Diff>& other) const {
		if (size() < other.size()) {
			return true;
		}
		if (size() > other.size()) {
			return false;
		}
		const_iterator b1 = begin();
		const_iterator b2 = other.begin();
		const_iterator e1 = end();
		while (b1 != e1) {
			if ((*b1) < (*b2)) {
				return true;
			}
			if ((*b2) < (*b1)) {
				return false;
			}
			++b1;
			++b2;
		}
		return false;
	}
	bool operator!=(const prevector<N, T, Size, Diff>& other) const {
		return !(*this == other);
	}
	bool operator==(const prevector<N, T, Size, Diff>& other) const {
		if (other.size() != size()) {
			return false;
		}
		const_iterator b1 = begin();
		const_iterator b2 = other.begin();
		const_iterator e1 = end();
		while (b1 != e1) {
			if ((*b1) != (*b2)) {
				return false;
			}
			++b1;
			++b2;
		}
		return true;
	}

	const type_of_value* data() const {
		return item_ptr(0);
	}
	type_of_value* data() {
		return item_ptr(0);
	}

	size_t allocated_memory() const {
		if (is_direct()) {
			return 0;
		}
		else {
			return ((size_t)(sizeof(T))) * _union.capacity;
		}
	}

private:
	void change_capacity(type_of_size new_capacity) {
		if (new_capacity <= N) {
			if (!is_direct()) {
				T* indirect = indirect_ptr(0);
				T* src = indirect;
				T* dst = direct_ptr(0);
				memcpy(dst, src, size() * sizeof(T));
				free(indirect);
				_size -= N + 1;
			}
		}
		else {
			if (!is_direct()) {
				_union.indirect = static_cast<char*>(realloc(_union.indirect, ((size_t)sizeof(T)) * new_capacity));
				assert(_union.indirect);
				_union.capacity = new_capacity;
			}
			else {
				char* new_indirect = static_cast<char*>(malloc(((size_t)sizeof(T)) * new_capacity));
				assert(new_indirect);
				T* src = direct_ptr(0);
				T* dst = reinterpret_cast<T*>(new_indirect);
				memcpy(dst, src, size() * sizeof(T));
				_union.indirect = new_indirect;
				_union.capacity = new_capacity;
				_size += N + 1;
			}
		}
	}

	const T* item_ptr(type_of_difference pos) const { return is_direct() ? direct_ptr(pos) : indirect_ptr(pos); }
	T* item_ptr(type_of_difference pos) { return is_direct() ? direct_ptr(pos) : indirect_ptr(pos); }

	bool is_direct() const { return _size <= N; }
	const T* direct_ptr(type_of_difference pos) const { return reinterpret_cast<const T*>(_union.direct) + pos; }
	T* direct_ptr(type_of_difference pos) { return reinterpret_cast<T*>(_union.direct) + pos; }
	const T* indirect_ptr(type_of_difference pos) const { return reinterpret_cast<const T*>(_union.indirect) + pos; }
	T* indirect_ptr(type_of_difference pos) { return reinterpret_cast<T*>(_union.indirect) + pos; }

private:
	type_of_size _size;
	union direct_or_indirect {
		char direct[sizeof(T) * N];
		struct {
			type_of_size capacity;
			char* indirect;
		};
	} _union;
};
#pragma pack(pop)

#endif
                                                                   