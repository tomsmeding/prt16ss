#pragma once

#include <cstddef>

/*
Either one type, or another. Mostly used to be able to return "either
a value, or an error string", in which case it is some form of
Either<string,T>. The "good" value should always be the right one, if
applicable, because that should be the "right one". For obvious reasons.
The error value then becomes left.
*/

template <typename T,typename U>
class Either{
	const T *left;
	const U *right;

	Either(T *left,U *right) noexcept;
public:
	~Either() noexcept;
	Either(T value) noexcept; //makes a Left-value for you
	Either(U value) noexcept; //makes a Right-value for you

	static Either<T,U> Left(T value) noexcept; //makes a Left-value for you
	static Either<T,U> Right(U value) noexcept; //makes a Right-value for you

	T fromLeft(void) const noexcept; //gets the Left-value, assuming it is one
	U fromRight(void) const noexcept; //gets the Right-value, assuming it is one

	bool isLeft(void) const noexcept; //query whether this is a Left-value
	bool isRight(void) const noexcept; //query whether this is a Right-value
};

template <typename T,typename U>
Either<T,U>::Either(T *left,U *right) noexcept
	:left(left),right(right){}

template <typename T,typename U>
Either<T,U>::~Either() noexcept{
	if(left)delete left;
	if(right)delete right;
}

template <typename T,typename U>
Either<T,U>::Either(T value) noexcept
	:left(new T(move(value))),right(nullptr){}

template <typename T,typename U>
Either<T,U>::Either(U value) noexcept
	:left(nullptr),right(new U(move(value))){}

template <typename T,typename U>
Either<T,U> Either<T,U>::Left(T value) noexcept {
	return Either<T,U>(new T(move(value)),nullptr);
}

template <typename T,typename U>
Either<T,U> Either<T,U>::Right(U value) noexcept {
	return Either<T,U>(nullptr,new U(move(value)));
}


template <typename T,typename U>
T Either<T,U>::fromLeft() const noexcept {
	return *left;
}

template <typename T,typename U>
U Either<T,U>::fromRight() const noexcept {
	return *right;
}

template <typename T,typename U>
bool Either<T,U>::isLeft() const noexcept {
	return (bool)left;
}

template <typename T,typename U>
bool Either<T,U>::isRight() const noexcept {
	return (bool)right;
}
