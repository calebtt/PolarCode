#pragma once
#include "stdafx.h"
#include "PolarInfoTypes.h"

namespace sds
{
	class PolarTransformMag
	{
	private:
		//using ComputationFloat_t = PolarInfoTypes::ComputationFloat_t;
		//using StickValue_t = PolarInfoTypes::StickValue_t;
		//using PolarInfoPack = PolarInfoTypes::PolarInfoPack;
		//using QuadrantInfoPack = PolarInfoTypes::QuadrantInfoPack;
		//using AdjustedMagnitudePack = PolarInfoTypes::AdjustedMagnitudePack;
		//using PolarCompleteInfoPack = PolarInfoTypes::PolarCompleteInfoPack;

		//number of coordinate plane quadrants (obviously 4)
		static constexpr auto NUM_QUADRANTS = 4;
		static constexpr double MY_PI{ std::numbers::pi };
		static constexpr double MY_PI2{ std::numbers::pi / 2.0 };
		static constexpr StickValue_t MAG_SENT_DEFAULT{ 32'766 };
	private:
		//array of boundary values, used to determine which polar quadrant a polar angle resides in
		static constexpr std::array<std::tuple<double, double>, NUM_QUADRANTS> m_quadArray
		{
			std::make_tuple(0.0, MY_PI2),
			std::make_tuple(MY_PI2, MY_PI),
			std::make_tuple(-MY_PI, -MY_PI2),
			std::make_tuple(-MY_PI2, 0.0)
		};
		//static constexpr std::array<const std::pair<double, double>, NUM_QUADRANTS> m_quadArray{
		//	std::make_pair(0.0, MY_PI2),
		//	std::make_pair(MY_PI2, MY_PI),
		//	std::make_pair(-MY_PI, -MY_PI2),
		//	std::make_pair(-MY_PI2, 0.0)
		//};
		// Holds the polar info computed at construction.
		PolarCompleteInfoPack ComputedInfo{};
		StickValue_t m_magSentinel{ MAG_SENT_DEFAULT };
	public:
		/// <summary> Ctor, constructs polar quadrant calc object. </summary>
		///	<param name="xStickVal"> <b>x axis hardware</b> value from thumbstick </param>
		///	<param name="yStickVal"> <b>y axis hardware</b> value from thumbstick </param>
		///	<param name="magSentinel"> Trim value for maximum magnitude, returned magnitude will be trimmed to this. </param>
		PolarTransformMag(const StickValue_t xStickVal, const StickValue_t yStickVal, const StickValue_t magSentinel = 32'766) noexcept : m_magSentinel(magSentinel)
		{
			ComputedInfo = ComputePolarCompleteInfo(static_cast<ComputationFloat_t>(xStickVal), static_cast<ComputationFloat_t>(yStickVal));
		}

		[[nodiscard]]
		auto get() const noexcept
		{
			return ComputedInfo;
		}
	private:
		[[nodiscard]] auto ComputePolarCompleteInfo(const ComputationFloat_t xStickValue, const ComputationFloat_t yStickValue) const noexcept -> PolarCompleteInfoPack
		{
			PolarCompleteInfoPack tempPack{};
			tempPack.polar_info = ComputePolarPair(xStickValue, yStickValue);
			tempPack.quadrant_info = GetQuadrantInfo(tempPack.polar_info.polar_theta_angle);
			tempPack.adjusted_magnitudes = ComputeAdjustedMagnitudes(tempPack.polar_info, tempPack.quadrant_info);
			return tempPack;
		}
		//compute adjusted magnitudes
		[[nodiscard]] auto ComputeAdjustedMagnitudes(const PolarInfoPack polarInfo, const QuadrantInfoPack quadInfo) const noexcept -> AdjustedMagnitudePack
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
		[[nodiscard]] auto ComputePolarPair(const ComputationFloat_t xStickValue, const ComputationFloat_t yStickValue) const noexcept -> PolarInfoPack
		{
			constexpr auto nonZeroValue{ std::numeric_limits<ComputationFloat_t>::min() }; // cannot compute with both values at 0, this is used instead
			const bool areBothZero = IsFloatZero(xStickValue) && IsFloatZero(yStickValue);

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
		[[nodiscard]] auto GetQuadrantInfo(const ComputationFloat_t polarTheta) const noexcept -> QuadrantInfoPack
		{
			int index{};
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
		[[nodiscard]] constexpr auto TrimMagnitudeToSentinel(const int x, const int y) const noexcept -> AdjustedMagnitudePack
		{
			using std::clamp;
			auto tempX = x;
			auto tempY = y;
			tempX = clamp(tempX, -m_magSentinel, m_magSentinel);
			tempY = clamp(tempY, -m_magSentinel, m_magSentinel);
			return { tempX, tempY };
		}

		[[nodiscard]] constexpr bool IsFloatZero(const auto testFloat) const noexcept
		{
			using std::abs, std::numeric_limits;
			constexpr auto eps = numeric_limits<decltype(testFloat)>::epsilon();
			constexpr auto eps2 = eps * 2;
			return abs(testFloat) <= eps2;
		}
	};
}
