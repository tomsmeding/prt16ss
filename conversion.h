#pragma once

#include "maybe.h"
#include <string>

using namespace std;

/*
A conversion function from string to some basic types. Used when making a
CellValueBasic.
*/

template <typename T>
Maybe<T> convertstring(string s) noexcept;
