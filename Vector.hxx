/* Copyright 2015, 2016 Robert Haberlach
	Distributed under the Boost Software License, Version 1.0.
	(See accompanying file LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt) */

#pragma once

#include "impl/BasicVector.hxx"

namespace Constainer {

/**< This is the fundamental class template that provides a 'resizable array'.
     Its interface is STD::vector-like. */
template <typename T, STD::size_t MaxN, typename CopyTraits>
class BasicVector :
	public detail::BasicVector<T, MaxN, CopyTraits, 0> {
	using  detail::BasicVector<T, MaxN, CopyTraits, 0>::BasicVector;
};

template <typename T, STD::size_t N1, STD::size_t N2, typename Traits1, typename Traits2>
constexpr void swap( BasicVector<T, N1, Traits1>& lhs, BasicVector<T, N2, Traits2>& rhs ) {
	lhs.swap(rhs);
}

// TODO: Implement comparison of BasicVector and Array

template <typename T1, STD::size_t Size1, typename Traits1,
          typename T2, STD::size_t Size2, typename Traits2>
constexpr bool operator==(BasicVector<T1, Size1, Traits1> const& lhs, BasicVector<T2, Size2, Traits2> const& rhs) {
	return lhs.size() == rhs.size() && Constainer::equal(lhs.begin(), lhs.end(), rhs.begin());
}
template <typename T1, STD::size_t Size1, typename Traits1,
          typename T2, STD::size_t Size2, typename Traits2>
constexpr bool operator<(BasicVector<T1, Size1, Traits1> const& lhs, BasicVector<T2, Size2, Traits2> const& rhs) {
	return Constainer::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}
template <typename T1, STD::size_t Size1, typename Traits1,
          typename T2, STD::size_t Size2, typename Traits2>
constexpr bool operator!=(BasicVector<T1, Size1, Traits1> const& lhs, BasicVector<T2, Size2, Traits2> const& rhs) {
	return !(lhs == rhs);
}
template <typename T1, STD::size_t Size1, typename Traits1,
          typename T2, STD::size_t Size2, typename Traits2>
constexpr bool operator>=(BasicVector<T1, Size1, Traits1> const& lhs, BasicVector<T2, Size2, Traits2> const& rhs) {
	return !(lhs < rhs);
}
template <typename T1, STD::size_t Size1, typename Traits1,
          typename T2, STD::size_t Size2, typename Traits2>
constexpr bool operator>(BasicVector<T1, Size1, Traits1> const& lhs, BasicVector<T2, Size2, Traits2> const& rhs) {
	return rhs < lhs;
}
template <typename T1, STD::size_t Size1, typename Traits1,
          typename T2, STD::size_t Size2, typename Traits2>
constexpr bool operator<=(BasicVector<T1, Size1, Traits1> const& lhs, BasicVector<T2, Size2, Traits2> const& rhs) {
	return !(lhs > rhs);
}

template <typename T, STD::size_t N=defaultContainerSize>
using Vector = BasicVector<T, N, DefaultCopyTraits<T>>;

}
