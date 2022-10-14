#pragma once
#include "stdafx.h"
#include <functional>

namespace sds
{
	template<int MagnitudeSentinel = 32'766>
	class PolarTransform
	{
	private:
		//number of coordinate plane quadrants (obviously 4)
		static constexpr auto NUM_QUADRANTS = 4;
		// Computation float type
		using ComputationFloat_t = double;
		// Stick input value type
		using StickValue_t = int;
		static constexpr ComputationFloat_t MY_PI{ std::numbers::pi };
		static constexpr ComputationFloat_t MY_PI2{ std::numbers::pi / 2.0 };
	private:
		// [Pair[FloatingType,FloatingType], int] wherein the quadrant_range pair is the quadrant range, and the outer int quadrant_number is the quadrant number.
		struct QuadrantInfoPack
		{
			std::pair<ComputationFloat_t, ComputationFloat_t> quadrant_range{};
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
	private:
		//array of boundary values, used to determine which polar quadrant a polar angle resides in
		static constexpr std::array<const std::pair<ComputationFloat_t, ComputationFloat_t>, NUM_QUADRANTS> m_quadArray{
			std::make_pair(0.0, MY_PI2),
			std::make_pair(MY_PI2, MY_PI),
			std::make_pair(-MY_PI, -MY_PI2),
			std::make_pair(-MY_PI2, 0.0)
		};
		// Holds the polar info computed at construction.
		PolarCompleteInfoPack ComputedInfo{};
	public:
		/// <summary> Ctor, constructs polar quadrant calc object. </summary>
		///	<param name="stickXValue"> <b>x axis hardware</b> value from thumbstick </param>
		///	<param name="stickYValue"> <b>y axis hardware</b> value from thumbstick </param>
		PolarTransform(
			const StickValue_t stickXValue,
			const StickValue_t stickYValue
		) noexcept
		{
			ComputedInfo = ComputePolarCompleteInfo(stickXValue, stickYValue);
		}
		/// <summary>
		/// Returns the polar info computed at construction.
		/// </summary>
		auto get() const noexcept
		{
			return ComputedInfo;
		}
	private:
		[[nodiscard]] PolarCompleteInfoPack ComputePolarCompleteInfo(const ComputationFloat_t xStickValue, const ComputationFloat_t yStickValue) noexcept
		{
			PolarCompleteInfoPack tempPack{};
			tempPack.polar_info = ComputePolarPair(xStickValue, yStickValue);
			tempPack.quadrant_info = GetQuadrantInfo(tempPack.polar_info.polar_theta_angle);
			tempPack.adjusted_magnitudes = ComputeAdjustedMagnitudes(tempPack.polar_info, tempPack.quadrant_info);
			return tempPack;
		}
		//compute adjusted magnitudes
		[[nodiscard]] AdjustedMagnitudePack ComputeAdjustedMagnitudes(const PolarInfoPack polarInfo, const QuadrantInfoPack quadInfo) const noexcept
		{
			const auto& [polarRadius, polarTheta] = polarInfo;
			const auto& [quadrantSentinelPair, quadrantNumber] = quadInfo;
			const auto& [quadrantBeginVal, quadrantSentinelVal] = quadrantSentinelPair;
			//compute proportion of the radius for each axis to be the axial magnitudes, apparently a per-quadrant calculation with my setup.
			const auto redPortion = (polarTheta - quadrantBeginVal) * polarRadius;
			const auto blackPortion = (quadrantSentinelVal - polarTheta) * polarRadius;
			const double xProportion = quadrantNumber % 2 ? blackPortion : redPortion;
			const double yProportion = quadrantNumber % 2 ? redPortion : blackPortion;
			return TrimMagnitudeToSentinel(static_cast<StickValue_t>(xProportion), static_cast<StickValue_t>(yProportion));
		}
		//compute polar coord pair
		[[nodiscard]] PolarInfoPack ComputePolarPair(const ComputationFloat_t xStickValue, const ComputationFloat_t yStickValue) const noexcept
		{
			constexpr ComputationFloat_t nonZeroValue{ 0.01 }; // cannot compute with both values at 0, this is used instead
			const bool areBothZero = xStickValue == decltype(xStickValue){}
									&& yStickValue == decltype(yStickValue){};

			const double xValue = areBothZero ? nonZeroValue : xStickValue;
			const double yValue = areBothZero ? nonZeroValue : yStickValue;
			const auto xSquared = xValue * xValue;
			const auto ySquared = yValue * yValue;
			const auto rad = std::sqrt(xSquared + ySquared);
			const auto angle = std::atan2(yValue, xValue);
			return { .polar_radius = rad, .polar_theta_angle = angle };
		}
		/// <summary> Retrieves begin and end range values for the quadrant the polar theta (angle) value resides in, and the quadrant number (NOT zero indexed!) </summary>
		/// <returns> Pair[Pair[double,double], int] wherein the inner pair is the quadrant range, and the outer int is the quadrant number. </returns>
		[[nodiscard]] QuadrantInfoPack GetQuadrantInfo(const ComputationFloat_t polarTheta) noexcept
		{
			constexpr std::string_view BAD_QUAD = "Invalid value that does not map to a quadrant in GetQuadrantInfo(const FloatingType)";
			size_t index{};
			//Find polar theta value's place in the quadrant range array.
			const auto quadrantResult = std::ranges::find_if(m_quadArray, [&](const auto val)
				{
					++index;
					return (polarTheta >= std::get<0>(val) && polarTheta <= std::get<1>(val));
				});
			//This should not happen, but if it does, I want some kind of message about it.
			assert(quadrantResult != m_quadArray.end());
			return { .quadrant_range = (*quadrantResult), .quadrant_number = static_cast<int>(index) };
		}
	private:
		//trim computed magnitude values to sentinel value
		[[nodiscard]] constexpr AdjustedMagnitudePack TrimMagnitudeToSentinel(const int x, const int y) const noexcept
		{
			auto tempX = x;
			auto tempY = y;
			tempX = std::clamp(tempX, -MagnitudeSentinel, MagnitudeSentinel);
			tempY = std::clamp(tempY, -MagnitudeSentinel, MagnitudeSentinel);
			return { tempX, tempY };
		}
	};
}
