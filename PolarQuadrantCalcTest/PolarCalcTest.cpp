#include "pch.h"
#include "CppUnitTest.h"
#include <chrono>
#include <format>
#include <string>
#include <sstream>

#include "../PolarQuadrantCalc/PolarAlgorithms.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace PolarQuadrantCalcTest
{
	static constexpr int MagnitudeMax{ 32'766 };
	static constexpr int MaxIterations{ 1'000 };

	[[nodiscard]] constexpr bool IsWithin(const auto result, const auto testVal, const auto within) noexcept
	{
		return ((result > (testVal - within)) && (result < (testVal + within)));
	}

	inline void PrintDurationUnit(const auto t1, const auto t2, const auto RunCount) noexcept
	{
		std::stringstream ss, ssNs, ssUs;
		const std::chrono::duration<double, std::milli> fp_ms = t2 - t1;
		const std::chrono::duration<double, std::nano> fp_nano = (t2 - t1) / RunCount;
		const std::chrono::duration<double, std::micro> fp_micro = (t2 - t1) / RunCount;
		ss << fp_ms;
		ssNs << fp_nano;
		ssUs << fp_micro;
		//ss << std::chrono::duration_cast<ChronoType_t> (t2 - t1);
		//const auto timeResult = std::chrono::duration_cast<ChronoType_t> (timeResultEnd - timeResultBegin);
		Logger::WriteMessage(std::format("Total time at {0} ...\n", ss.str().c_str()).c_str());
		Logger::WriteMessage(std::format("Avg time per computation/iteration {0} ...\n", ssNs.str().c_str()).c_str());
		Logger::WriteMessage(std::format("Avg time per computation/iteration {0} ...\n", ssUs.str().c_str()).c_str());
	}

	TEST_CLASS(PolarCalcTest)
	{
		inline static constexpr bool PrintMessages{ false };
		static constexpr char newl{ '\n' };

		static
		auto AssertValues(
			const auto xMax, 
			const auto yMax, 
			const auto x, 
			const auto y,
			const auto xResult,
			const auto yResult)
		{
			if constexpr (PrintMessages)
			{
				std::wstringstream ws1, ws2;
				std::stringstream ss1, ss2;
				ws1 << L"X axis not within acceptable parameters: ";
				ss1 << xMax << newl;
				std::string ts1{ ss1.str() };
				ws1 << std::wstring{ ts1.begin(), ts1.end() };
				ws1 << L"Inputs were: x:" << x << " y:" << y << newl;

				ws2 << L"Y axis not within acceptable parameters:  ";
				ss2 << yMax << newl;
				std::string ts2{ ss2.str() };
				ws2 << std::wstring{ ts2.begin(), ts2.end() };

				Assert::IsTrue(xResult, ws1.str().c_str());
				Assert::IsTrue(yResult, ws2.str().c_str());
			}
			else
			{
				Assert::IsTrue(xResult);
				Assert::IsTrue(yResult);
			}
		}


		static void AssertXYEqual(const short SMax, auto& pc)
		{
			const auto completeInfo = pc.ComputePolarCompleteInfo(SMax / 2, SMax / 2);
			const auto [xMax, yMax] = completeInfo.adjusted_magnitudes;
			Assert::AreEqual(static_cast<int>(xMax), static_cast<int>(yMax), L"");
		}

	public:

		TEST_METHOD(TestFreeFunctions)
		{
			static constexpr auto WITHIN_VAL{ 2000 };

			using std::wstring, std::wcout, std::cout;
			using namespace std::string_literals;
			using Clock_t = std::chrono::steady_clock;

			//theoretical hardware value max
			constexpr auto SMax = std::numeric_limits<SHORT>::max();
			//theoretical hardware value max
			constexpr auto SMin = std::numeric_limits<SHORT>::min();

			//lambda for testing
			auto ComputeAndShow = [&](const int x, const int y, std::string_view msg, const int xShouldBe, const int yShouldBe)
				{
					//polar transform instance
					const auto pc = sds::ComputePolarCompleteInfo(x, y);
					const auto [xMax, yMax] = pc.adjusted_magnitudes;

					if constexpr (PrintMessages) 
					{
						std::stringstream ss;
						ss << "Magnitude of x at " << msg << ": " << xMax << newl;
						ss << "Magnitude of y at " << msg << ": " << yMax << newl;
						ss << "-------" << newl;
						Logger::WriteMessage(ss.str().c_str());
					}

					const auto xResult = IsWithin(xMax, xShouldBe, WITHIN_VAL);
					const auto yResult = IsWithin(yMax, yShouldBe, WITHIN_VAL);
					AssertValues(xMax, yMax, x, y, xResult, yResult);
				};
			ComputeAndShow(SMax, SMax, "short-max,short-max", MagnitudeMax, MagnitudeMax);
			ComputeAndShow(SMin, SMax, "short-min,short-max", MagnitudeMax, MagnitudeMax);
			ComputeAndShow(SMin, SMin, "short-min,short-min", MagnitudeMax, MagnitudeMax);
			ComputeAndShow(SMin / 2, SMin / 2, "short-min/2,short-min/2", (SMax) / 2, (SMax) / 2);

			const auto [xMax, yMax] = sds::ComputePolarCompleteInfo<MagnitudeMax>(SMax / 2, SMax / 2).adjusted_magnitudes;
			Assert::AreEqual(static_cast<int>(xMax), static_cast<int>(yMax), L"");

			//Running several iterations in a loop with begin and end time stamps.
			const auto timeResultBegin = Clock_t::now();
			for (size_t i = 0; i < MaxIterations; ++i)
			{
				ComputeAndShow(SMax, SMax, "short-max,short-max", MagnitudeMax, MagnitudeMax);
				ComputeAndShow(SMin, SMax, "short-min,short-max", MagnitudeMax, MagnitudeMax);
				ComputeAndShow(SMin, SMin, "short-min,short-min", MagnitudeMax, MagnitudeMax);
				ComputeAndShow(SMin / 2, SMin / 2, "short-min/2,short-min/2", (SMax) / 2, (SMax) / 2);
			}
			const auto timeResultEnd = Clock_t::now();
			std::stringstream msg1, msg2;
			msg1 << "Starting at " << timeResultBegin.time_since_epoch() << '\n';
			msg2 << "Ending at " << timeResultEnd.time_since_epoch() << '\n';
			Logger::WriteMessage(msg1.str().c_str());
			Logger::WriteMessage(msg2.str().c_str());
			PrintDurationUnit(timeResultBegin, timeResultEnd, MaxIterations);
		}

	};
}
