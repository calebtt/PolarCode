#pragma once
#include "stdafx.h"
#include "PolarInfoTypes.h"
#include <functional>
#include <cassert>

namespace sds
{
	namespace detail
	{
		//number of coordinate plane quadrants (obviously 4)
		static constexpr auto NUM_QUADRANTS = 4;
		static constexpr double MY_PI{ std::numbers::pi };
		static constexpr double MY_PI2{ std::numbers::pi / 2.0 };

		//array of boundary values, used to determine which polar quadrant a polar angle resides in
		static constexpr std::array<std::tuple<double, double>, NUM_QUADRANTS> PolarQuadrantArray
		{
			std::make_tuple(0.0, MY_PI2),
			std::make_tuple(MY_PI2, MY_PI),
			std::make_tuple(-MY_PI, -MY_PI2),
			std::make_tuple(-MY_PI2, 0.0)
		};

		//trim computed magnitude values to sentinel value
		template<int MagnitudeSentinel = 32'766>
		[[nodiscard]] constexpr auto TrimMagnitudeToSentinel(const int x, const int y) noexcept -> AdjustedMagnitudePack
		{
			return { std::clamp(x, -MagnitudeSentinel, MagnitudeSentinel), std::clamp(y, -MagnitudeSentinel, MagnitudeSentinel) };
		}

		[[nodiscard]] constexpr bool IsFloatZero(const auto testFloat) noexcept
		{
			using std::abs;
			using std::numeric_limits;
			constexpr auto eps = numeric_limits<decltype(testFloat)>::epsilon();
			constexpr auto eps2 = eps * 2;
			return abs(testFloat) <= eps2;
		}

		template<typename T1>
		[[nodiscard]] constexpr auto Clamp(const auto& val, const T1& minVal, const T1& maxVal) noexcept
		{
			if (val < minVal)
				return decltype(val){minVal};
			if (val > maxVal)
				return decltype(val){maxVal};
			return val;
		}

	}

	//compute adjusted magnitudes
	template<int MagnitudeSentinel = 32'766>
	[[nodiscard]] constexpr auto ComputeAdjustedMagnitudes(PolarInfoPack polarInfo, QuadrantInfoPack quadInfo) noexcept -> AdjustedMagnitudePack
	{
		const auto& [polarRadius, polarTheta] = polarInfo;
		const auto& [quadrantSentinelPair, quadrantNumber] = quadInfo;
		const auto& [quadrantBeginVal, quadrantSentinelVal] = quadrantSentinelPair;
		//compute proportion of the radius for each axis to be the axial magnitudes, apparently a per-quadrant calculation with my setup.
		const auto redPortion = (polarTheta - quadrantBeginVal) * polarRadius;
		const auto blackPortion = (quadrantSentinelVal - polarTheta) * polarRadius;
		const double xProportion = quadrantNumber % 2 ? blackPortion : redPortion;
		const double yProportion = quadrantNumber % 2 ? redPortion : blackPortion;
		return detail::TrimMagnitudeToSentinel<MagnitudeSentinel>(static_cast<StickValue_t>(xProportion), static_cast<StickValue_t>(yProportion));
	}

	//compute polar coord pair, returns (polar_radius, polar_theta angle)
	[[nodiscard]] inline auto ComputePolarPair(const ComputationFloat_t xStickValue, const ComputationFloat_t yStickValue) noexcept -> PolarInfoPack
	{
		constexpr auto nonZeroValue{ std::numeric_limits<ComputationFloat_t>::min() }; // cannot compute with both values at 0, this is used instead
		const bool areBothZero = detail::IsFloatZero(xStickValue) && detail::IsFloatZero(yStickValue);

		const double xValue = areBothZero ? nonZeroValue : xStickValue;
		const double yValue = areBothZero ? nonZeroValue : yStickValue;
		const auto xSquared = xValue * xValue;
		const auto ySquared = yValue * yValue;
		const auto rad = std::sqrt(xSquared + ySquared);
		const auto angle = std::atan2(yValue, xValue);
		return { .polar_radius = rad, .polar_theta_angle = angle };
	}

	/// <summary> Retrieves begin and end range values for the quadrant the polar theta (angle) value resides in, and the quadrant number (NOT zero indexed!) </summary>
	///	<returns> Pair[Pair[double,double], int] wherein the inner pair is the quadrant range, and the outer int is the quadrant number. </returns>
	[[nodiscard]] inline auto GetQuadrantInfo(const ComputationFloat_t polarTheta) noexcept -> QuadrantInfoPack
	{
		int index{};
		//Find polar theta value's place in the quadrant range array.
		const auto quadrantResult = std::ranges::find_if(detail::PolarQuadrantArray, [&](const auto val)
			{
				++index;
				return (polarTheta >= std::get<0>(val) && polarTheta <= std::get<1>(val));
			});
		//This should not happen, but if it does, I want some kind of message about it.
		assert(quadrantResult != PolarQuadrantArray.end());
		return { .quadrant_range = (*quadrantResult), .quadrant_number = static_cast<int>(index) };
	}

	template<int MagnitudeSentinel = 32'766>
	[[nodiscard]] inline auto ComputePolarCompleteInfo(const ComputationFloat_t xStickValue, const ComputationFloat_t yStickValue) noexcept -> PolarCompleteInfoPack
	{
		// Ordered computation of results below.
		const auto polarInfo = ComputePolarPair(xStickValue, yStickValue);
		const auto quadInfo = GetQuadrantInfo(polarInfo.polar_theta_angle);
		const auto adjustedMagnitudes = ComputeAdjustedMagnitudes<MagnitudeSentinel>(polarInfo, quadInfo);

		return PolarCompleteInfoPack
		{
			.polar_info = polarInfo,
			.quadrant_info = quadInfo,
			.adjusted_magnitudes = adjustedMagnitudes
		};
	}

}
