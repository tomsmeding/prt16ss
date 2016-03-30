#pragma once

#include "maybe.h"
#include "util.h"
#include <string>
#include <stdexcept>

using namespace std;

template <typename T>
Maybe<T> convertstring(string s);
