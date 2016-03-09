#pragma once

#include "either.h"

template <typename T>             //to be used as, for example, ErrorOr<double>: either an error (Left(string)) or
using ErrorOr = Either<string,T>; //a Right(double) -- remember as: Right is the right value, Left is the wrong value

using Errtype = Maybe<string>; //if there can be an error or nothing
