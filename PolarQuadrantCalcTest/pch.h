// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <array>
#include <locale>
#include <sstream>
#include <chrono>
#include <tuple>
#include <numeric>
#include <typeinfo>
#include <exception>
#include <format>
#include <iomanip>
#include <utility>
#include <concepts>
#include <bitset>
#include <ostream>
#include <sstream>
#include <syncstream>
#include <thread>
#include <concepts>
#include <ranges>
#include <iostream>
#include <variant>
#include <map>
#include <filesystem>


#endif //PCH_H
