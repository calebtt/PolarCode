#pragma once
#include "stdafx.h"
#include <type_traits>
#include <concepts>
#include <functional>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
//#include <boost/multiprecision/gmp.hpp>
//#include <boost/multiprecision/mpfr.hpp>

namespace sds
{
	namespace bmp = boost::multiprecision;

	template<
		typename ComputationFloat_t = float,
		typename StickValue_t = int>
	class PolarTransformFloat
	{
	private:
		using BigFloat = bmp::cpp_dec_float_50;
	private:
		//number of coordinate plane quadrants (obviously 4)
		static constexpr int NUM_QUADRANTS = 4;
		static constexpr float MY_PI{ std::numbers::pi_v<float> };
		static constexpr float MY_PI2{ std::numbers::pi_v<float> / 2.0f };
		int m_magnitudeSentinel = 32'766;
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
		static constexpr std::array<const std::pair<float, float>, NUM_QUADRANTS> m_quadArray{
			std::make_pair(0.0f, MY_PI2),
			std::make_pair(MY_PI2, MY_PI),
			std::make_pair(-MY_PI, -MY_PI2),
			std::make_pair(-MY_PI2, 0.0f)
		};
		// Holds the polar info computed at construction.
		PolarCompleteInfoPack ComputedInfo{};
	public:
		/// <summary> Ctor, constructs polar quadrant calc object. </summary>
		///	<param name="stickXValue"> <b>x axis hardware</b> value from thumbstick </param>
		///	<param name="stickYValue"> <b>y axis hardware</b> value from thumbstick </param>
		PolarTransformFloat(
			const StickValue_t stickXValue,
			const StickValue_t stickYValue
		) noexcept
		{
			ComputedInfo = ComputePolarCompleteInfo(stickXValue, stickYValue);
		}
		/// <summary> Ctor, constructs polar quadrant calc object. </summary>
		///	<param name="stickXValue"> <b>x axis hardware</b> value from thumbstick </param>
		///	<param name="stickYValue"> <b>y axis hardware</b> value from thumbstick </param>
		///	<param name="magSentinel">Magnitude sentinel to trim magnitude values to.</param>
		PolarTransformFloat(
			const StickValue_t stickXValue,
			const StickValue_t stickYValue,
			const int magSentinel
		) noexcept : m_magnitudeSentinel(magSentinel)
		{
			ComputedInfo = ComputePolarCompleteInfo(stickXValue, stickYValue);
		}
		/// <summary>
		/// Returns the polar info computed at construction.
		/// </summary>
		[[nodiscard]] auto get() const noexcept
		{
			return ComputedInfo;
		}
	private:
		[[nodiscard]]
		auto ComputePolarCompleteInfo(const StickValue_t xStickValue, const StickValue_t yStickValue) noexcept -> PolarCompleteInfoPack
		{
			PolarCompleteInfoPack tempPack{};
			tempPack.polar_info = ComputePolarPair(xStickValue, yStickValue);
			tempPack.quadrant_info = GetQuadrantInfo(tempPack.polar_info.polar_theta_angle);
			tempPack.adjusted_magnitudes = ComputeAdjustedMagnitudes(tempPack.polar_info, tempPack.quadrant_info);
			return tempPack;
		}
		//compute polar coord pair
		[[nodiscard]]
		auto ComputePolarPair(const StickValue_t xStickValue, const StickValue_t yStickValue) const noexcept -> PolarInfoPack
		{
			const auto xValue = GetPositiveNonZero(xStickValue);
			const auto yValue = GetPositiveNonZero(yStickValue);
			BigFloat xSquared = xValue * xValue;
			BigFloat ySquared = yValue * yValue;
			const auto rad = GetSqrtOfSum(std::move(xSquared), std::move(ySquared));
			const auto angle = GetAtan2(yValue, xValue);
			return { .polar_radius = static_cast<ComputationFloat_t>(rad), .polar_theta_angle = static_cast<ComputationFloat_t>(angle) };
		}
		/// <summary> Retrieves begin and end range values for the quadrant the polar theta (angle) value resides in, and the quadrant number (NOT zero indexed!) </summary>
		/// <returns> Pair[Pair[double,double], int] wherein the inner pair is the quadrant range, and the outer int is the quadrant number. </returns>
		[[nodiscard]]
		constexpr
		auto GetQuadrantInfo(const ComputationFloat_t polarTheta) const noexcept -> QuadrantInfoPack
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
			return { .quadrant_range = (*quadrantResult), .quadrant_number = index};
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
			const float xProportion = quadrantNumber % 2 ? blackPortion : redPortion;
			const float yProportion = quadrantNumber % 2 ? redPortion : blackPortion;
			assert(xProportion >= 0.0f);
			assert(yProportion >= 0.0f);
			return TrimMagnitudeToSentinel(static_cast<StickValue_t>(xProportion), static_cast<StickValue_t>(yProportion));
		}
	private:
		//trim computed magnitude values to sentinel value
		[[nodiscard]]
		constexpr
		auto TrimMagnitudeToSentinel(const auto x, const auto y) const noexcept -> AdjustedMagnitudePack
		{
			const auto tempX = x;
			const auto tempY = y;
			const auto tempXClamped = std::clamp(tempX, -m_magnitudeSentinel, m_magnitudeSentinel);
			const auto tempYClamped = std::clamp(tempY, -m_magnitudeSentinel, m_magnitudeSentinel);
			return { tempXClamped, tempYClamped };
		}
		[[nodiscard]]
		static
		constexpr
		auto GetEpsOrOne() noexcept requires std::integral<StickValue_t>
		{
			return 1;
		}
		[[nodiscard]]
		static
		constexpr
		auto GetEpsOrOne() noexcept requires std::floating_point<StickValue_t>
		{
			return std::numeric_limits<StickValue_t>::epsilon();
		}
		[[nodiscard]]
		static
		constexpr
		auto GetPositiveNonZero(const auto val) noexcept
		{
			auto v = std::abs(val);
			if (v == decltype(val){})
				return GetEpsOrOne();
			return v;
		}

		[[nodiscard]]
		static
		constexpr
		auto GetSqrtOfSum(BigFloat xVal, BigFloat yVal)
		{
			return bmp::sqrt(xVal + yVal);
		}

		[[nodiscard]]
		static
		constexpr
		auto GetAtan2(BigFloat yVal, BigFloat xVal)
		{
			return bmp::atan2(yVal, xVal);
		}
	};
}
