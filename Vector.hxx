#ifndef VECTOR_HXX_INCLUDED
#define VECTOR_HXX_INCLUDED

#include "Array.hxx"
#include "Assert.hxx"

namespace Constainer {

// inb4 you can drop the private y'know
template <typename T, std::size_t MaxN>
class Vector : private Array<T, MaxN> {

	using _base = Array<T, MaxN>;

public:
	using typename _base::size_type;
	using typename _base::value_type;
	using typename _base::iterator;
	using typename _base::const_iterator;
	using typename _base::reverse_iterator;
	using typename _base::const_reverse_iterator;
	using typename _base::reference;
	using typename _base::const_reference;

private:
	size_type _size = 0;

	constexpr void _verifySize() const {
		AssertExcept<std::bad_alloc>( size() <= MaxN );
	}
	constexpr void _sizeIncable(size_type s = 1) const {
		AssertExcept<std::bad_alloc>( size() <= MaxN-s );
	}
	constexpr void _verifiedSizeInc() {
		++_size; _verifySize();
	}

public:

	using _base::rend;
	using _base::crend;
	using _base::begin;
	using _base::cbegin;
	using _base::front;
	static constexpr auto max_size() {return _base::size();}

	constexpr auto size() const {return _size;}
	constexpr bool empty() const {return size() == 0;}

	constexpr Vector() :_base{},  _size(0) {}

	constexpr Vector( size_type s ) : _base{}, _size(s) {_verifySize();}

	constexpr Vector( size_type s, value_type const& v ) : Vector(s) {
		fill_n( this->begin(), size(), v );
	}

	constexpr const_reverse_iterator crbegin() const {return rbegin();}
	constexpr const_iterator            cend() const {return    end();}
	constexpr reverse_iterator        rbegin()       {return       reverse_iterator(end());}
	constexpr const_reverse_iterator  rbegin() const {return const_reverse_iterator(end());}
	constexpr iterator                   end()       {return this->begin() + size();}
	constexpr const_iterator             end() const {return this->begin() + size();}

	constexpr       reference back ()       {return end()[-1];}
	constexpr const_reference back () const {return end()[-1];}

	constexpr reference       operator[](size_type s) {
		Assert(s < size(), "Invalid index!"); return _base::operator[](s);
	}
	constexpr const_reference operator[](size_type s) const {
		Assert(s < size(), "Invalid index!"); return _base::operator[](s);
	}

	template <typename... Args>
	constexpr void emplace_back( Args&&... args ) {
		push_back( value_type(std::forward<Args>(args)...) );
	}

private:
	template <typename U>
	constexpr void _push_back( U&& u ) {
		_sizeIncable();
		*this->end() = std::forward<U>(u);
		++_size;
	}

public:

	constexpr void push_back( value_type const& v ) {
		_push_back(v);
	}
	constexpr void push_back( value_type&& v ) {
		_push_back(std::move(v));
	}

	constexpr void erase( const_iterator first, const_iterator last ) {
		move( const_cast<iterator>(last),    this->end(),
		      const_cast<iterator>(first));
		_size -= last-first;
	}
	constexpr void erase( const_iterator it ) {
		erase(it, it+1);
	}

	template <typename InputIterator>
	constexpr void insert( const_iterator pos, InputIterator first, InputIterator last ) {
		auto size_increase = distance(first, last);
		_sizeIncable(size_increase);
		move_backward( const_cast<iterator>(pos), end(), end()+size_increase );
		copy( first, last, const_cast<iterator>(pos) );
		_size += size_increase;
	}

	constexpr void insert( const_iterator i, value_type const& r ) {
		insert(i, &r, &r+1);
	}

	constexpr void insert( const_iterator i, std::initializer_list<value_type> ilist ) {
		insert(i, ilist.begin(), ilist.end());
	}

	constexpr void pop_back() {Assert(size() >= 1, "Can't pop"); erase(end()-1);}

	template <typename U, std::size_t OtherMax>
	constexpr void swap( Vector<U, OtherMax>& other ) {
		Assert( other.size() < max_size() && size() < OtherMax, "Swap fails" );

		auto min = std::min(other.size(), size());
		swap_ranges(begin(other), begin(other) + min, begin());

		if (other.size()> size())
			move(begin(other) + min, end(other), begin     ()+min);
		else
			move(begin     () + min, end     (), begin(other)+min);

		swap(_size, other._size);
	}
};

}

#endif // VECTOR_HXX_INCLUDED
