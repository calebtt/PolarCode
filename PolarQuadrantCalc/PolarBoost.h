#pragma once
#include "stdafx.h"
#include <functional>
#ifdef HAS_BOOST
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>


namespace sds
{
	template<
		int MagnitudeSentinel = 32'766,
		typename ComputationFloat_t = boost::multiprecision::cpp_bin_float_50,
		typename StickValue_t = boost::multiprecision::int128_t>
	class PolarBoost
	{
	private:
		//number of coordinate plane quadrants (obviously 4)
		static constexpr auto NUM_QUADRANTS = 4;
		static constexpr double MY_PI{ std::numbers::pi };
		static constexpr double MY_PI2{ std::numbers::pi / 2.0 };
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
		static constexpr std::array<const std::pair<double, double>, NUM_QUADRANTS> m_quadArray{
			std::make_pair(0.0, MY_PI2),
			std::make_pair(MY_PI2, MY_PI),
			std::make_pair(-MY_PI, -MY_PI2),
			std::make_pair(-MY_PI2, 0.0)
		};
		// Holds the polar info computed at construction.
		//PolarCompleteInfoPack ComputedInfo{};
	public:
		///// <summary> Ctor, constructs polar quadrant calc object. </summary>
		/////	<param name="stickXValue"> <b>x axis hardware</b> value from thumbstick </param>
		/////	<param name="stickYValue"> <b>y axis hardware</b> value from thumbstick </param>
		//PolarBoost(
		//	const StickValue_t stickXValue,
		//	const StickValue_t stickYValue
		//) noexcept
		//{
		//	ComputedInfo = ComputePolarCompleteInfo( stickXValue.convert_to<ComputationFloat_t>() ,  stickYValue.convert_to<ComputationFloat_t>() );
		//}
		///// <summary>
		///// Returns the polar info computed at construction.
		///// </summary>
		//[[nodiscard]] auto get() const noexcept
		//{
		//	return ComputedInfo;
		//}
	
		[[nodiscard]]
		static
		auto ComputePolarCompleteInfo(const ComputationFloat_t& xStickValue, const ComputationFloat_t& yStickValue) noexcept -> PolarCompleteInfoPack
		{
			PolarCompleteInfoPack tempPack{};
			tempPack.polar_info = ComputePolarPair(xStickValue, yStickValue);
			tempPack.quadrant_info = GetQuadrantInfo(tempPack.polar_info.polar_theta_angle);
			tempPack.adjusted_magnitudes = ComputeAdjustedMagnitudes(tempPack.polar_info, tempPack.quadrant_info);
			return tempPack;
		}
	private:
		//compute adjusted magnitudes
		[[nodiscard]]
		static
		auto ComputeAdjustedMagnitudes(const PolarInfoPack polarInfo, const QuadrantInfoPack quadInfo) noexcept -> AdjustedMagnitudePack
		{
			const auto& [polarRadius, polarTheta] = polarInfo;
			const auto& [quadrantSentinelPair, quadrantNumber] = quadInfo;
			const auto& [quadrantBeginVal, quadrantSentinelVal] = quadrantSentinelPair;
			//compute proportion of the radius for each axis to be the axial magnitudes, apparently a per-quadrant calculation with my setup.
			const auto redPortion = (polarTheta - quadrantBeginVal) * polarRadius;
			const auto blackPortion = (quadrantSentinelVal - polarTheta) * polarRadius;
			const auto xProportion = quadrantNumber % 2 ? blackPortion : redPortion;
			const auto yProportion = quadrantNumber % 2 ? redPortion : blackPortion;
			const auto rx = static_cast<StickValue_t>(xProportion);
			const auto ry = static_cast<StickValue_t>(yProportion);
			return TrimMagnitudeToSentinel(rx, ry);
		}
		//compute polar coord pair
		[[nodiscard]]
		static
		auto ComputePolarPair(const ComputationFloat_t xStickValue, const ComputationFloat_t yStickValue) noexcept -> PolarInfoPack
		{
			const auto nonZeroValue{ std::numeric_limits<ComputationFloat_t>::min() }; // cannot compute with both values at 0, this is used instead
			const bool areBothZero = IsFloatZero(xStickValue) && IsFloatZero(yStickValue);

			const auto xValue = areBothZero ? nonZeroValue : xStickValue;
			const auto yValue = areBothZero ? nonZeroValue : yStickValue;
			const auto xSquared = xValue * xValue;
			const auto ySquared = yValue * yValue;
			const auto rad = sqrt(ComputationFloat_t(xSquared + ySquared));
			const auto angle = atan2(yValue, xValue);
			return { .polar_radius = rad, .polar_theta_angle = angle };
		}
		/// <summary> Retrieves begin and end range values for the quadrant the polar theta (angle) value resides in, and the quadrant number (NOT zero indexed!) </summary>
		/// <returns> Pair[Pair[double,double], int] wherein the inner pair is the quadrant range, and the outer int is the quadrant number. </returns>
		[[nodiscard]]
		static
		auto GetQuadrantInfo(const ComputationFloat_t polarTheta) noexcept -> QuadrantInfoPack
		{
			int index{};
			//Find polar theta value's place in the quadrant range array.
			const auto quadrantResult = std::ranges::find_if(m_quadArray, [&](const auto& val)
				{
					++index;
					// Real check
					return (polarTheta >= std::get<0>(val) && polarTheta <= std::get<1>(val));
				});
			//This should not happen, but if it does, I want some kind of message about it.
			assert(quadrantResult != m_quadArray.end());
			return { .quadrant_range = (*quadrantResult), .quadrant_number = static_cast<int>(index) };
		}
	private:
		//trim computed magnitude values to sentinel value
		[[nodiscard]]
		static
		constexpr
		auto TrimMagnitudeToSentinel(const auto& x, const auto& y) noexcept -> AdjustedMagnitudePack
		{
			auto tempX = x;
			auto tempY = y;
			tempX = Clamp(tempX, -MagnitudeSentinel, MagnitudeSentinel);
			tempY = Clamp(tempY, -MagnitudeSentinel, MagnitudeSentinel);
			return { tempX, tempY };
		}
		[[nodiscard]]
		static
		bool IsFloatZero(const auto testFloat) noexcept
		{
			const auto eps = std::numeric_limits< std::remove_cv_t<decltype(testFloat)> >::epsilon();
			const auto eps2 = eps * 2;
			return abs(testFloat) <= eps2;
		}
		template<typename T1>
		[[nodiscard]]
		constexpr
		static
		auto Clamp(const auto& val, const T1& minVal, const T1& maxVal) noexcept
		{
			if (val < minVal)
				return decltype(val){minVal};
			if (val > maxVal)
				return decltype(val){maxVal};
			return val;
		}
	};

	using PolarCalcBoost = PolarBoost<>;
}
#endif