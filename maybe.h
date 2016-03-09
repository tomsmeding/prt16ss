#pragma once

#include <cstddef>
#include <utility>

using namespace std;

struct Nothing{}; //sentinel value for Maybe

template <typename T>
class Maybe{
	const T *val;

public:
	Maybe(T value); //constructs a Just(v)
	Maybe(Nothing); //constructs an empty Maybe

	~Maybe();

	T fromJust() const; //gets the Just-value from the container, assuming it isn't Nothing
	T fromMaybe(T &def) const; //gets the Just-value if there is one, or a copy of `def` if it's Nothing

	bool isJust() const; //query whether this is a Just-value
	bool isNothing() const; //query whether this is Nothing (==!isJust())
};

template <typename T>
Maybe<T>::Maybe(T value)
	:val(new T(move(value))){}

template <typename T>
Maybe<T>::Maybe(Nothing)
	:val(nullptr){}

template <typename T>
Maybe<T>::~Maybe(){
	if(val)delete val;
}


template <typename T>
T Maybe<T>::fromJust() const {
	return *val;
}

template <typename T>
T Maybe<T>::fromMaybe(T &def) const {
	return val?*val:def;
}

template <typename T>
bool Maybe<T>::isJust() const {
	return (bool)val;
}

template <typename T>
bool Maybe<T>::isNothing() const {
	return !val;
}
