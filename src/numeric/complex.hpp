#pragma once
#ifndef _COMPLEX_HPP
#define _COMPLEX_HPP

#include <type_traits>
#include <complex>

template<typename T> struct is_complex : std::false_type {};
template<typename T> struct is_complex<std::complex<T>> : std::true_type {};

namespace numeric
{

template<typename T> inline T conj(const T& c)
{ return c; }
template<typename T> inline std::complex<T> conj(const std::complex<T>& c)
{ return std::conj<T>(c); }

}

#endif /* _COMPLEX_HPP */
