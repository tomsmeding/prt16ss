// Tom Smeding (s1685694), Tim Brouwer (s1663615), Ruben Turkenburg (s1659685)

#pragma once

#include <cstddef>
#include <utility> //for move()

using namespace std;

/*
Either a value of some type, or nothing. This could have been implemented
as, and is functionally equivalent to, Either<Nothing,T>, but a separate
class arguably enables the use of much better method names.
*/

struct Nothing{}; //special empty value for use with Maybe

template <typename T>
class Maybe{
	const T *val;

public:
	Maybe(T value) noexcept; //constructs a Just(v)
	Maybe(Nothing) noexcept; //constructs an empty Maybe

	~Maybe() noexcept;

	//gets the Just-value, assuming it isn't Nothing
	T fromJust() const noexcept;

	//gets the Just-value if present, or a copy of `def` if it's Nothing
	T fromMaybe(T &def) const noexcept;

	bool isJust() const noexcept; //query whether this is a Just-value
	bool isNothing() const noexcept; //query whether this is Nothing (==!isJust())
};

template <typename T>
Maybe<T>::Maybe(T value) noexcept
	:val(new T(move(value))){}

template <typename T>
Maybe<T>::Maybe(Nothing) noexcept
	:val(nullptr){}

template <typename T>
Maybe<T>::~Maybe() noexcept {
	if(val)delete val;
}


template <typename T>
T Maybe<T>::fromJust() const noexcept {
	return *val;
}

template <typename T>
T Maybe<T>::fromMaybe(T &def) const noexcept {
	return val?*val:def;
}

template <typename T>
bool Maybe<T>::isJust() const noexcept {
	return (bool)val;
}

template <typename T>
bool Maybe<T>::isNothing() const noexcept {
	return !val;
}
