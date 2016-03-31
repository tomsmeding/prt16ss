#pragma once

#include <cstddef>

template <typename T,typename U>
class Either{
	const T *left;
	const U *right;

	Either(T *left,U *right);
public:
	~Either();
	Either(T value); //makes a Left-value for you
	Either(U value); //makes a Right-value for you

	static Either<T,U> Left(T value); //makes a Left-value for you
	static Either<T,U> Right(U value); //makes a Right-value for you

	T fromLeft(void) const; //gets the Left-value, assuming it is one
	U fromRight(void) const; //gets the Right-value, assuming it is one

	bool isLeft(void) const; //query whether this is a Left-value
	bool isRight(void) const; //query whether this is a Right-value
};

template <typename T,typename U>
Either<T,U>::Either(T *left,U *right)
	:left(left),right(right){}

template <typename T,typename U>
Either<T,U>::~Either(){
	if(left)delete left;
	if(right)delete right;
}

template <typename T,typename U>
Either<T,U>::Either(T value)
	:left(new T(move(value))),right(nullptr){}

template <typename T,typename U>
Either<T,U>::Either(U value)
	:left(nullptr),right(new U(move(value))){}

template <typename T,typename U>
Either<T,U> Either<T,U>::Left(T value){
	return Either<T,U>(new T(move(value)),nullptr);
}

template <typename T,typename U>
Either<T,U> Either<T,U>::Right(U value){
	return Either<T,U>(nullptr,new U(move(value)));
}


template <typename T,typename U>
T Either<T,U>::fromLeft() const {
	return *left;
}

template <typename T,typename U>
U Either<T,U>::fromRight() const {
	return *right;
}

template <typename T,typename U>
bool Either<T,U>::isLeft() const {
	return (bool)left;
}

template <typename T,typename U>
bool Either<T,U>::isRight() const {
	return (bool)right;
}
