/* Copyright 2015, 2016 Robert Haberlach
	Distributed under the Boost Software License, Version 1.0.
	(See accompanying file LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt) */

#pragma once

#include "../Array.hxx"
#include "../Assert.hxx"

#include <algorithm>

namespace Constainer {

template <typename T>
struct DefaultCopyTraits {
	using value_type = T;
	using pointer = T*;
	using const_pointer = T const*;
	using reference = T&;
	using const_reference = T const&;

	static constexpr pointer copy(pointer s1, const_pointer s2, STD::size_t n) {
		return Constainer::copy_n(s2, n, s1);
	}

	/**< This function is called to actually 'move', not copy. */
	static constexpr pointer move(pointer s1, const_pointer s2, STD::size_t n) {
		return Constainer::move_n(s2, n, s1);
	}

	static constexpr pointer assign(pointer s, STD::size_t n, const_reference a) {
		return Constainer::fill_n(s, n, a);
	}

	/**< Called after erase has performed on elements in the container */
	static constexpr void destroy(pointer, STD::size_t) {}

	template <typename U>
	static constexpr void assign( reference r, U&& a ) {
		r = STD::forward<U>(a);
	}
};

}

namespace Constainer { namespace detail{

template <typename T, STD::size_t MaxN, typename CopyTraits, STD::size_t addBufferSize>
class BasicVector : private Array<T, MaxN+addBufferSize> {

	template <typename, STD::size_t, typename, STD::size_t>
	friend class BasicVector;

	using _base = Array<T, MaxN+addBufferSize>;

public:
	using traits_type = CopyTraits;

	using typename _base::difference_type;
	using typename _base::size_type;
	using typename _base::value_type;
	using typename _base::iterator;
	using typename _base::const_iterator;
	using typename _base::pointer;
	using typename _base::const_pointer;
	using typename _base::reverse_iterator;
	using typename _base::const_reverse_iterator;
	using typename _base::reference;
	using typename _base::const_reference;

protected:
	constexpr pointer _address(const_iterator i) {
		static_assert( STD::is_same<const_iterator, const_pointer>{},
		               "Requires const_iterator = const_pointer" );
		return const_cast<pointer>(i);
	}

	constexpr pointer _remcv(const_iterator i) {
		static_assert( STD::is_same<const_iterator, const_pointer>{},
		               "Requires const_iterator = const_pointer" );
		return const_cast<iterator>(i);
	}

private:
	template <size_type OtherN>
	using ThisResized = BasicVector<value_type, OtherN, traits_type, addBufferSize>;

protected:

	size_type _size = 0;

	constexpr auto _data() {return this->_storage;}

	constexpr void _verifySizeInc(size_type n) const {
		AssertExcept<STD::length_error>( size() <= MaxN-n, "Invalid attempt to increase container size" );
	}
	constexpr void _verifiedSizeInc(size_type n) {
		_verifySizeInc(n); _size += n;
	}

	constexpr void _createInsertionSpace(const_iterator pos, size_type len) {
		assert(pos <= end());
		_verifiedSizeInc(len);
		if (pos != end()-len)
			// TODO: Implement move_backward that uses CopyTraits::assign
			Constainer::move_backward(_address(pos), end()-len, end());
	}

public:
	using _base::data;

	using _base::rend;
	using _base::crend;
	using _base::begin;
	using _base::cbegin;
	using _base::front;
	static constexpr auto max_size() {return _base::size();}

	constexpr auto size() const {return _size;}
	constexpr bool empty() const {return size() == 0;}

	constexpr BasicVector() :_base{},  _size(0) {}

	constexpr explicit BasicVector( size_type n ) : _base{}, _size(n) {
		_verifySizeInc(0); // The INCREASE is 0, not the size to test!
	}

	constexpr BasicVector( size_type n, value_type const& v ) : BasicVector(0) {
		insert(begin(), n, v);
	}

	template <typename InputIt,
	          typename=require<isInputIterator<InputIt>>>
	constexpr BasicVector( InputIt first, InputIt last ) : BasicVector() {
		assign(first, last);
	}

private:
	constexpr BasicVector(const_pointer p, size_type l, STD::false_type) : BasicVector(l) {
		traits_type::copy(_data(), p, l);
	}
	constexpr BasicVector(const_pointer p, size_type l, STD::true_type) : BasicVector(l) {
		traits_type::move(_data(), p, l);
	}

	template <STD::size_t OtherN>
	constexpr BasicVector(ThisResized<OtherN> const& other, int) :
		BasicVector(other.data(), other.size(), STD::false_type()) {}

	template <STD::size_t OtherN>
	constexpr BasicVector(ThisResized<OtherN> && other, int) :
		BasicVector(other.data(), other.size(), STD::true_type())  {
		other.clear();
	}

public:

	template <STD::size_t OtherN>
	constexpr BasicVector(ThisResized<OtherN> const& other) : BasicVector(other, 0) {}
	template <STD::size_t OtherN>
	constexpr BasicVector(ThisResized<OtherN>     && other) : BasicVector(STD::move(other), 0) {}

	constexpr BasicVector(BasicVector const& other) : BasicVector(other, 0) {}
	constexpr BasicVector(BasicVector&& other)      : BasicVector(STD::move(other), 0) {}

	constexpr BasicVector(STD::initializer_list<value_type> ilist)
		: BasicVector(STD::begin(ilist), STD::end(ilist)) {}

	template <STD::size_t OtherN>
	constexpr BasicVector& operator=( ThisResized<OtherN> const& other ) {
		AssertExcept<STD::bad_alloc>( other.size() <= max_size() );
		assign(other.begin(), other.end());
		return *this;
	}

	template <STD::size_t OtherN>
	constexpr BasicVector& operator=( ThisResized<OtherN>&& other ) {
		AssertExcept<STD::bad_alloc>( other.size() <= max_size() );
		assign(make_move_iterator(other.begin()), make_move_iterator(other.end()));
		other.clear();
		return *this;
	}

	constexpr BasicVector& operator=( BasicVector const& other )
	{ return operator=<>(other); }

	constexpr BasicVector& operator=( BasicVector&& other )
	{ return operator=<>(STD::move(other)); }

	constexpr BasicVector& operator=( STD::initializer_list<value_type> ilist )
	{ assign(ilist); return *this; }

	constexpr const_reverse_iterator crbegin() const {return rbegin();}
	constexpr const_iterator            cend() const {return    end();}
	constexpr reverse_iterator        rbegin()       {return       reverse_iterator(end());}
	constexpr const_reverse_iterator  rbegin() const {return const_reverse_iterator(end());}
	constexpr iterator                   end()       {return this->begin() + size();}
	constexpr const_iterator             end() const {return this->begin() + size();}

	constexpr       reference back()       {return end()[-1];}
	constexpr const_reference back() const {return end()[-1];}

	constexpr reference       operator[](size_type i) {
		assert(i < size() && "Invalid index!"); return _base::operator[](i);
	}
	constexpr const_reference operator[](size_type i) const {
		assert(i < size() && "Invalid index!"); return _base::operator[](i);
	}

private:

	/**< This overload of emplace_back is viable if the shown way to
	     initialize a temporary of value_type is valid */
	template <typename... Args>
	constexpr auto _emplace_back( int, Args&&... args )
	  -> decltype( void(value_type(STD::forward<Args>(args)...)) ) {
		push_back( value_type(STD::forward<Args>(args)...) );
	}
	/**< … otherwise, try direct-list-initialization */
	template <typename... Args>
	constexpr void _emplace_back( float, Args&&... args ) {
		push_back( value_type{STD::forward<Args>(args)...} );
	}

	template <typename U>
	constexpr void _push_back( U&& u ) {
		_verifySizeInc(1);
		traits_type::assign(*this->end(), STD::forward<U>(u));
		++_size;
	}

public:

	template <typename... Args>
	constexpr void emplace_back( Args&&... args ) {
		_emplace_back( 0, STD::forward<Args>(args)... );
	}

	constexpr void push_back( value_type const& v ) {
		_push_back(v);
	}
	constexpr void push_back( value_type&& v ) {
		_push_back(STD::move(v));
	}

	constexpr iterator erase( const_iterator first, const_iterator last ) {
		traits_type::move( _address(first), _address(last), end() - last);
		traits_type::destroy(end() - (last-first), last-first);
		_size -= last-first;
		return _remcv(first);
	}
	constexpr iterator erase( const_iterator it ) {
		return erase(it, it+1);
	}

	constexpr void clear() {erase(begin(), end());}

private:
	template <typename InputIt>
	constexpr iterator _insert( iterator pos, InputIt first, InputIt last, STD::input_iterator_tag ) {
		while (first != last)
			insert(pos++, *first++);
		return pos;
	}

	template <typename InputIt>
	constexpr iterator _insert_n( iterator pos, size_type n, InputIt first ) {
		_createInsertionSpace(pos, n);
		copy_n( first, n, pos );
		return pos;
	}
	template <typename ForwardIt>
	constexpr iterator _insert( iterator pos, ForwardIt first, ForwardIt last, STD::forward_iterator_tag ) {
		return _insert_n(pos, Constainer::distance(first, last), first);
	}

	template <typename U>
	constexpr iterator _insert( const_iterator pos, U&& u ) {
		auto it = _remcv(pos);
		_createInsertionSpace(it, 1);
		traits_type::assign(*it, STD::forward<U>(u));
		return it;
	}

public:
	template <typename... Args>
	constexpr iterator emplace( const_iterator i, Args&&... args ) {
		// TODO: Avoid unnecessary copy (?)
		return insert(i, value_type(STD::forward<Args>(args)...));
	}

	template <typename InputIt>
	constexpr require<isInputIterator<InputIt>, iterator>
	insert( const_iterator pos, InputIt first, InputIt last ) {
		return _insert(_remcv(pos), first, last,
		               typename STD::iterator_traits<InputIt>::iterator_category{});
	}

	template <typename InputIt>
	constexpr require<isInputIterator<InputIt>, iterator>
	insert( const_iterator pos, size_type n, InputIt first ) {
		return _insert_n(_remcv(pos), n, first);
	}

	constexpr iterator insert( const_iterator pos, value_type const& v ) {return _insert(pos, v);}
	constexpr iterator insert( const_iterator pos, value_type     && v ) {return _insert(pos, STD::move(v));}

	constexpr iterator insert( const_iterator pos, size_type n, const_reference v ) {
		_createInsertionSpace(pos, n);
		traits_type::assign(_remcv(pos), n, v);
		return _remcv(pos);
	}

	constexpr iterator insert( const_iterator i, STD::initializer_list<value_type> ilist ) {
		return insert(i, ilist.begin(), ilist.end());
	}

	constexpr void assign( STD::initializer_list<value_type> ilist ) {
		clear(); insert(ilist);
	}

	constexpr void assign( size_type n, const_reference value ) {
		clear();
		insert(begin(), n, value);
	}

	template <class InputIt>
	constexpr require<isInputIterator<InputIt>>
	assign(InputIt first, InputIt last) {
		clear();
		while (first != last)
			emplace_back(*first++);
	}

	constexpr void pop_back() {assert(not empty() && "Can't pop"); erase(end()-1);}

public:

	template <STD::size_t OtherMax, typename OtherTraits, STD::size_t addBuffOther>
	constexpr void swap( BasicVector<value_type, OtherMax, OtherTraits, addBuffOther>& other ) {
		AssertExcept<STD::out_of_range>( other.size() < max_size() && size() < OtherMax, "Swap fails" );

		auto min = STD::min(other.size(), size());
		// TODO: Use traits_type::assign calls to swap the ranges
		Constainer::swap_ranges(other.begin(), other.begin() + min, begin());

		// Use the trait of the destination string
		if (other.size() > size())
			traits_type::move(_address(begin()+min), _address(other.begin()+min), other.end() - (other.begin() + min));
		else
			OtherTraits::move(_address(other.begin()+min), _address(begin()+min), end() - (begin() + min));

		Constainer::swap(_size, other._size);
	}

	constexpr void resize(size_type n, const_reference value) {
		if (n < size())
			erase(begin()+n, end());
		else if (n > size())
			insert(end(), n-size(), value);
	}
	constexpr void resize(size_type n) {
		resize(n, value_type());
	}
};

}

}
