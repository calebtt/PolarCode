#pragma once
#include "stdafx.h"
#include <type_traits>
#include <concepts>
#include <functional>
#include <tuple>
#include <optional>

namespace sds
{
	using ComputationFloat_t = double;
	using StickValue_t = int;

	// [Pair[FloatingType,FloatingType], int] wherein the quadrant_range pair is the quadrant range, and the outer int quadrant_number is the quadrant number.
	struct QuadrantInfoPack
	{
		std::tuple<ComputationFloat_t, ComputationFloat_t> quadrant_range{};
		int quadrant_number{};
	};

	//Pair[FloatingType, FloatingType] wherein the first member is the polar radius, and the second is the polar theta angle.
	struct PolarInfoPack
	{
		ComputationFloat_t polar_radius{};
		ComputationFloat_t polar_theta_angle{};
	};

	//Pair[FloatingType, FloatingType] wherein the first member is the adjusted X magnitude value, and the second is the adjusted Y magnitude
	struct AdjustedMagnitudePack
	{
		StickValue_t x_adjusted_mag{};
		StickValue_t y_adjusted_mag{};
	};

	struct PolarCompleteInfoPack
	{
		PolarInfoPack polar_info{};
		QuadrantInfoPack quadrant_info{};
		AdjustedMagnitudePack adjusted_magnitudes{};
	};

}