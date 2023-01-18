#pragma once
#include "stdafx.h"
#include <functional>
#include <expected>
#include <optional>

namespace sds
{
	template<
		int MagnitudeSentinel = 32'766,
		class ComputationFloat_t = double,
		class StickValue_t = int>
	class PolarTransformOptional
	{
		// Type for bounds of quadrant theta
		using QuadRange_t = float;
		// Type for polar radius and theta
		using PolarInfo_t = float;
	private:
		//number of coordinate plane quadrants (obviously 4)
		static constexpr auto NUM_QUADRANTS = 4;
		static constexpr QuadRange_t MY_PI{ std::numbers::pi_v<float> };
		static constexpr QuadRange_t MY_PI2{ std::numbers::pi_v<float> / 2.0f };
	private:
		// [Pair[FloatingType,FloatingType], int] wherein the quadrant_range pair is the quadrant range, and the outer int quadrant_number is the quadrant number.
		struct QuadrantInfoPack
		{
			std::pair<QuadRange_t, QuadRange_t> quadrant_range{};
			int quadrant_number{};

			QuadrantInfoPack() = default;
			QuadrantInfoPack(std::pair<QuadRange_t, QuadRange_t> qr, int num) : quadrant_range(qr), quadrant_number(num) { }

			QuadrantInfoPack(const QuadrantInfoPack& other)
				: quadrant_range(other.quadrant_range),
				  quadrant_number(other.quadrant_number)
			{
			}

			QuadrantInfoPack(QuadrantInfoPack&& other) noexcept
				: quadrant_range(std::move(other.quadrant_range)),
				  quadrant_number(other.quadrant_number)
			{
			}

			QuadrantInfoPack& operator=(const QuadrantInfoPack& other)
			{
				if (this == &other)
					return *this;
				quadrant_range = other.quadrant_range;
				quadrant_number = other.quadrant_number;
				return *this;
			}

			QuadrantInfoPack& operator=(QuadrantInfoPack&& other) noexcept
			{
				if (this == &other)
					return *this;
				quadrant_range = std::move(other.quadrant_range);
				quadrant_number = other.quadrant_number;
				return *this;
			}
		};
		//Pair[FloatingType, FloatingType] wherein the first member is the polar radius, and the second is the polar theta angle.
		struct PolarInfoPack
		{
			PolarInfo_t polar_radius{};
			PolarInfo_t polar_theta_angle{};
		};
		//Pair[FloatingType, FloatingType] wherein the first member is the adjusted X magnitude value, and the second is the adjusted Y magnitude
		struct AdjustedMagnitudePack
		{
			//StickValue_t matches the type of the input stick values, appropriate.
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
		static constexpr std::array<const std::pair<QuadRange_t, QuadRange_t>, NUM_QUADRANTS> m_quadArray{
			std::make_pair(decltype(MY_PI2){}, MY_PI2),
			std::make_pair(MY_PI2, MY_PI),
			std::make_pair(-MY_PI, -MY_PI2),
			std::make_pair(-MY_PI2, decltype(MY_PI2){})
		};
		// Holds the polar info computed at construction.
		PolarCompleteInfoPack ComputedInfo{};
	public:
		/// <summary> Ctor, constructs polar quadrant calc object. </summary>
		///	<param name="stickXValue"> <b>x axis hardware</b> value from thumbstick </param>
		///	<param name="stickYValue"> <b>y axis hardware</b> value from thumbstick </param>
		PolarTransformOptional(
			const StickValue_t stickXValue,
			const StickValue_t stickYValue
		) noexcept
		{
			ComputedInfo = ComputePolarCompleteInfo(static_cast<ComputationFloat_t>(stickXValue), static_cast<ComputationFloat_t>(stickYValue))
				.value_or(PolarCompleteInfoPack{});
		}
		/// <summary>
		/// Returns the polar info computed at construction.
		/// </summary>
		[[nodiscard]]
		auto get() const noexcept
		{
			return ComputedInfo;
		}
	private:
		[[nodiscard]]
		static
		auto ComputePolarCompleteInfo(const ComputationFloat_t xStickValue, const ComputationFloat_t yStickValue) noexcept
			-> std::optional<PolarCompleteInfoPack>
		{
			const auto pp = ComputePolarPair(xStickValue, yStickValue);
			[[likely]]
			if (pp)
			{
				const auto qp = GetQuadrantInfo(pp.value());
				[[likely]]
				if(qp)
				{
					const auto ap = ComputeAdjustedMagnitudes(pp.value(), qp.value());
					PolarCompleteInfoPack pcip{ pp.value(), qp.value(), ap };
					return PolarCompleteInfoPack{ pp.value(), qp.value(), ap};
				}
			}
			return {};
		}

		//compute polar coord pair
		[[nodiscard]]
		static
		auto ComputePolarPair(const ComputationFloat_t xStickValue, const ComputationFloat_t yStickValue) noexcept
			-> std::optional<PolarInfoPack>
		{
			const bool areBothZero = IsFloatZero(xStickValue) && IsFloatZero(yStickValue);
			[[unlikely]]
			if (areBothZero)
				return {};
			const auto& xValue = xStickValue;
			const auto& yValue = yStickValue;

			const auto xSquared = xValue * xValue;
			const auto ySquared = yValue * yValue;
			const auto rad = std::sqrt(xSquared + ySquared);
			const auto angle = std::atan2(yValue, xValue);

			return PolarInfoPack{ static_cast<PolarInfo_t>(rad), static_cast<PolarInfo_t>(angle) };
		}

		/// <summary> Retrieves begin and end range values for the quadrant the polar theta (angle) value resides in, and the quadrant number (NOT zero indexed!) </summary>
		///	<returns> Pair[Pair[double,double], int] wherein the inner pair is the quadrant range, and the outer int is the quadrant number. </returns>
		[[nodiscard]]
		static
		auto GetQuadrantInfo(const PolarInfoPack pack) noexcept
			-> std::optional<QuadrantInfoPack>
		{
			const auto& polarTheta = pack.polar_theta_angle;
			int index{};
			//Find polar theta value's place in the quadrant range array.
			const auto quadrantResult = std::ranges::find_if(m_quadArray, [&](const auto val)
				{
					++index;
			return (polarTheta >= std::get<0>(val) && polarTheta <= std::get<1>(val));
				});
			//This should not happen, but if it does, I want some kind of message about it.
			[[unlikely]]
			if (quadrantResult == m_quadArray.end())
				return {};
			return QuadrantInfoPack((*quadrantResult), static_cast<int>(index));
		}

		//compute adjusted magnitudes
		[[nodiscard]]
		static
		auto ComputeAdjustedMagnitudes(const PolarInfoPack pInfo, const QuadrantInfoPack pack) noexcept
			-> AdjustedMagnitudePack
		{
			const auto& [polarRadius, polarTheta] = pInfo;
			const auto& [quadrantSentinelPair, quadrantNumber] = pack;
			const auto& [quadrantBeginVal, quadrantSentinelVal] = quadrantSentinelPair;
			//compute proportion of the radius for each axis to be the axial magnitudes, apparently a per-quadrant calculation with my setup.
			const auto redPortion = (polarTheta - quadrantBeginVal) * polarRadius;
			const auto blackPortion = (quadrantSentinelVal - polarTheta) * polarRadius;
			const double xProportion = quadrantNumber % 2 ? blackPortion : redPortion;
			const double yProportion = quadrantNumber % 2 ? redPortion : blackPortion;
			return TrimMagnitudeToSentinel(static_cast<StickValue_t>(xProportion), static_cast<StickValue_t>(yProportion));
		}

	private:
		//trim computed magnitude values to sentinel value
		[[nodiscard]]
		static
		constexpr
		AdjustedMagnitudePack TrimMagnitudeToSentinel(const int x, const int y) noexcept
		{
			auto tempX = x;
			auto tempY = y;
			tempX = std::clamp(tempX, -MagnitudeSentinel, MagnitudeSentinel);
			tempY = std::clamp(tempY, -MagnitudeSentinel, MagnitudeSentinel);
			return { tempX, tempY };
		}
		[[nodiscard]]
		static
		bool IsFloatZero(const auto testFloat) noexcept
		{
			return testFloat == decltype(testFloat){};
		}
	};
}
