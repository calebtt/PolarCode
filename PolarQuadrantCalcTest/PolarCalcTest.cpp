#include "pch.h"
#include "CppUnitTest.h"
#include <chrono>
#include <format>
#include "../PolarQuadrantCalc/PolarCalc.h"
#include "../PolarQuadrantCalc/PolarCalcFaster.h"
#include "../PolarQuadrantCalc/PolarTransform.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace PolarQuadrantCalcTest
{
	TEST_CLASS(PolarCalcTest)
	{
		static constexpr auto MAGNITUDE_MAX{ 28'000.0 };

		void PrintDurationUnit(const auto t1, const auto t2) const noexcept
		{
			std::stringstream ss;
			const std::chrono::duration<double, std::milli> fp_ms = t2 - t1;
			ss << fp_ms;
			//ss << std::chrono::duration_cast<ChronoType_t> (t2 - t1);
			//const auto timeResult = std::chrono::duration_cast<ChronoType_t> (timeResultEnd - timeResultBegin);
			Logger::WriteMessage(std::format("Total time at {0} ...\n", ss.str().c_str()).c_str());
		}
	public:
		[[nodiscard]] static constexpr bool IsWithin(const auto result, const auto testVal, const auto within) noexcept
		{
			return ((result > (testVal - within)) && (result < (testVal + within)));
		}

		TEST_METHOD(TestComputeMagnitudes)
		{
			using std::wstring;
			using std::wcout;
			using std::cout;
			static constexpr char newl{ '\n' };
			static constexpr auto WITHIN_VAL{ 2000 };
			static constexpr size_t MAX_ITERATIONS{ 1'000'000 };
			//theoretical hardware value max
			constexpr auto SMax = std::numeric_limits<SHORT>::max();
			//theoretical hardware value max
			constexpr auto SMin = std::numeric_limits<SHORT>::min();
			//error logging fn
			auto LogFn = [this](const char* str) { Logger::WriteMessage(str); Assert::IsTrue(false); };
			//polar calc instance
			sds::PolarCalc pc(MAGNITUDE_MAX, LogFn);
			//lambda for testing
			auto ComputeAndShow = [&](const double x, const double y, const std::string& msg, const double xShouldBe, const double yShouldBe, const bool printMessages = true)
			{
				const auto completeInfo = pc.ComputePolarCompleteInfo(x, y);
				const auto [xMax, yMax] = completeInfo.adjusted_magnitudes;
				std::stringstream ss;
				ss << "Magnitude of x at " + msg + ": " << xMax << newl;
				ss << "Magnitude of y at " + msg + ": " << yMax << newl;
				ss << "-------" << newl;
				if (printMessages)
					Logger::WriteMessage(ss.str().c_str());
				const auto xResult = IsWithin(xMax, xShouldBe, WITHIN_VAL);
				const auto yResult = IsWithin(yMax, yShouldBe, WITHIN_VAL);
				Assert::IsTrue(xResult, L"X axis not within acceptable parameters...");
				Assert::IsTrue(yResult, L"Y axis not within acceptable parameters...");
			};
			ComputeAndShow(SMax, SMax, "short-max,short-max", MAGNITUDE_MAX, MAGNITUDE_MAX);
			ComputeAndShow(SMin, SMax, "short-min,short-max", MAGNITUDE_MAX, MAGNITUDE_MAX);
			ComputeAndShow(SMin, SMin, "short-min,short-min", MAGNITUDE_MAX, MAGNITUDE_MAX);
			ComputeAndShow(SMin / 2.0, SMin / 2.0, "short-min/2,short-min/2", (SMax) / 2.0, (SMax) / 2.0);
			const auto completeInfo = pc.ComputePolarCompleteInfo(SMax / 2.0, SMax / 2.0);
			const auto [xMax, yMax] = completeInfo.adjusted_magnitudes;
			Assert::AreEqual(static_cast<int>(xMax), static_cast<int>(yMax), L"");

			//Running several iterations in a loop with begin and end time stamps.
			const auto timeResultBegin = std::chrono::system_clock::now();
			for (size_t i = 0; i < MAX_ITERATIONS; ++i)
			{
				ComputeAndShow(SMax, SMax, "short-max,short-max", MAGNITUDE_MAX, MAGNITUDE_MAX, false);
				ComputeAndShow(SMin, SMax, "short-min,short-max", MAGNITUDE_MAX, MAGNITUDE_MAX, false);
				ComputeAndShow(SMin, SMin, "short-min,short-min", MAGNITUDE_MAX, MAGNITUDE_MAX, false);
				ComputeAndShow(SMin / 2.0, SMin / 2.0, "short-min/2,short-min/2", (SMax) / 2.0, (SMax) / 2.0, false);
			}
			const auto timeResultEnd = std::chrono::system_clock::now();
			Logger::WriteMessage(std::format("Starting at {0} ...\n", timeResultBegin).c_str());
			Logger::WriteMessage(std::format("Ending at {0} ...\n", timeResultEnd).c_str());
			PrintDurationUnit(timeResultBegin, timeResultEnd);
			//const auto timeResult = std::chrono::duration_cast<std::chrono::microseconds> (timeResultEnd - timeResultBegin);
			//Logger::WriteMessage(std::format("Total time at {0} ...\n", timeResult).c_str());
		}

		TEST_METHOD(TestComputeMagnitudesTransform)
		{
			// Test fn for the transform variant of the polar calc
			using std::wstring;
			using std::wcout;
			using std::cout;
			static constexpr char newl{ '\n' };
			static constexpr auto WITHIN_VAL{ 2000 };
			static constexpr size_t MAX_ITERATIONS{ 1'000'000 };
			static constexpr int MAGNITUDE_MAX_LOCAL{ 28'000 };
			//theoretical hardware value max
			constexpr auto SMax = std::numeric_limits<SHORT>::max();
			//theoretical hardware value max
			constexpr auto SMin = std::numeric_limits<SHORT>::min();
			//error logging fn
			auto LogFn = [this](const char* str) { Logger::WriteMessage(str); Assert::IsTrue(false); };
			
			//lambda for testing
			auto ComputeAndShow = [&](const int x, const int y, const std::string& msg, const int xShouldBe, const int yShouldBe, const bool printMessages = true)
			{
				//polar transform instance
				sds::PolarTransform pc(x, y, LogFn, static_cast<int>(MAGNITUDE_MAX_LOCAL));
				const auto completeInfo = pc.get();
				const auto [xMax, yMax] = completeInfo.adjusted_magnitudes;
				std::stringstream ss;
				ss << "Magnitude of x at " + msg + ": " << xMax << newl;
				ss << "Magnitude of y at " + msg + ": " << yMax << newl;
				ss << "-------" << newl;
				if (printMessages)
					Logger::WriteMessage(ss.str().c_str());
				const auto xResult = IsWithin(xMax, xShouldBe, WITHIN_VAL);
				const auto yResult = IsWithin(yMax, yShouldBe, WITHIN_VAL);
				Assert::IsTrue(xResult, L"X axis not within acceptable parameters...");
				Assert::IsTrue(yResult, L"Y axis not within acceptable parameters...");
			};
			ComputeAndShow(SMax, SMax, "short-max,short-max", MAGNITUDE_MAX_LOCAL, MAGNITUDE_MAX_LOCAL);
			ComputeAndShow(SMin, SMax, "short-min,short-max", MAGNITUDE_MAX_LOCAL, MAGNITUDE_MAX_LOCAL);
			ComputeAndShow(SMin, SMin, "short-min,short-min", MAGNITUDE_MAX_LOCAL, MAGNITUDE_MAX_LOCAL);
			ComputeAndShow(SMin / 2, SMin / 2, "short-min/2,short-min/2", (SMax) / 2, (SMax) / 2);
			//const auto completeInfo = sds::PolarTransform(SMax / 2, SMax / 2).get();
			const auto [xMax, yMax] = sds::PolarTransform(SMax / 2, SMax / 2).get().adjusted_magnitudes;
			Assert::AreEqual(static_cast<int>(xMax), static_cast<int>(yMax), L"");

			//Running several iterations in a loop with begin and end time stamps.
			const auto timeResultBegin = std::chrono::system_clock::now();
			for (size_t i = 0; i < MAX_ITERATIONS; ++i)
			{
				ComputeAndShow(SMax, SMax, "short-max,short-max", MAGNITUDE_MAX_LOCAL, MAGNITUDE_MAX_LOCAL, false);
				ComputeAndShow(SMin, SMax, "short-min,short-max", MAGNITUDE_MAX_LOCAL, MAGNITUDE_MAX_LOCAL, false);
				ComputeAndShow(SMin, SMin, "short-min,short-min", MAGNITUDE_MAX_LOCAL, MAGNITUDE_MAX_LOCAL, false);
				ComputeAndShow(SMin / 2, SMin / 2, "short-min/2,short-min/2", (SMax) / 2, (SMax) / 2, false);
			}
			const auto timeResultEnd = std::chrono::system_clock::now();
			Logger::WriteMessage(std::format("Starting at {0} ...\n", timeResultBegin).c_str());
			Logger::WriteMessage(std::format("Ending at {0} ...\n", timeResultEnd).c_str());
			PrintDurationUnit(timeResultBegin, timeResultEnd);
			//const auto timeResult = std::chrono::duration_cast<std::chrono::microseconds> (timeResultEnd - timeResultBegin);
			//Logger::WriteMessage(std::format("Total time at {0} ...\n", timeResult).c_str());
		}

		TEST_METHOD(TestComputeMagnitudesFaster)
		{
			using std::wstring;
			using std::wcout;
			using std::cout;
			static constexpr char newl{ '\n' };
			static constexpr auto WITHIN_VAL{ 2000 };
			static constexpr size_t MAX_ITERATIONS{ 1'000'000 };
			static constexpr int MAGNITUDE_MAX_LOCAL{ 28'000 };
			//theoretical hardware value max
			constexpr auto SMax = std::numeric_limits<SHORT>::max();
			//theoretical hardware value max
			constexpr auto SMin = std::numeric_limits<SHORT>::min();
			//error logging fn
			auto LogFn = [this](const char* str) { Logger::WriteMessage(str); Assert::IsTrue(false); };
			//polar calc instance
			sds::PolarCalcFaster pc(MAGNITUDE_MAX_LOCAL, LogFn);
			//lambda for testing
			auto ComputeAndShow = [&](const double x, const double y, const std::string& msg, const double xShouldBe, const double yShouldBe, const bool printMessages = true)
			{
				const auto completeInfo = pc.ComputePolarCompleteInfo(x, y);
				const auto [xMax, yMax] = completeInfo.adjusted_magnitudes;
				std::stringstream ss;
				ss << "Magnitude of x at " + msg + ": " << xMax << newl;
				ss << "Magnitude of y at " + msg + ": " << yMax << newl;
				ss << "-------" << newl;
				if (printMessages)
					Logger::WriteMessage(ss.str().c_str());
				const auto xResult = IsWithin(xMax, xShouldBe, WITHIN_VAL);
				const auto yResult = IsWithin(yMax, yShouldBe, WITHIN_VAL);
				Assert::IsTrue(xResult, L"X axis not within acceptable parameters...");
				Assert::IsTrue(yResult, L"Y axis not within acceptable parameters...");
			};
			ComputeAndShow(SMax, SMax, "short-max,short-max", MAGNITUDE_MAX_LOCAL, MAGNITUDE_MAX_LOCAL);
			ComputeAndShow(SMin, SMax, "short-min,short-max", MAGNITUDE_MAX_LOCAL, MAGNITUDE_MAX_LOCAL);
			ComputeAndShow(SMin, SMin, "short-min,short-min", MAGNITUDE_MAX_LOCAL, MAGNITUDE_MAX_LOCAL);
			ComputeAndShow(SMin / 2.0, SMin / 2.0, "short-min/2,short-min/2", (SMax) / 2.0, (SMax) / 2.0);
			const auto completeInfo = pc.ComputePolarCompleteInfo(SMax / 2.0, SMax / 2.0);
			const auto [xMax, yMax] = completeInfo.adjusted_magnitudes;
			Assert::AreEqual(static_cast<int>(xMax), static_cast<int>(yMax), L"");

			//Running several iterations in a loop with begin and end time stamps.
			const auto timeResultBegin = std::chrono::system_clock::now();
			for (size_t i = 0; i < MAX_ITERATIONS; ++i)
			{
				ComputeAndShow(SMax, SMax, "short-max,short-max", MAGNITUDE_MAX_LOCAL, MAGNITUDE_MAX_LOCAL, false);
				ComputeAndShow(SMin, SMax, "short-min,short-max", MAGNITUDE_MAX_LOCAL, MAGNITUDE_MAX_LOCAL, false);
				ComputeAndShow(SMin, SMin, "short-min,short-min", MAGNITUDE_MAX_LOCAL, MAGNITUDE_MAX_LOCAL, false);
				ComputeAndShow(SMin / 2.0, SMin / 2.0, "short-min/2,short-min/2", (SMax) / 2.0, (SMax) / 2.0, false);
			}
			const auto timeResultEnd = std::chrono::system_clock::now();
			Logger::WriteMessage(std::format("Starting at {0} ...\n", timeResultBegin).c_str());
			Logger::WriteMessage(std::format("Ending at {0} ...\n", timeResultEnd).c_str());
			PrintDurationUnit(timeResultBegin, timeResultEnd);
			//const auto timeResult = std::chrono::duration_cast<std::chrono::microseconds> (timeResultEnd - timeResultBegin);
			//Logger::WriteMessage(std::format("Total time at {0} ...\n", timeResult).c_str());
		}
	};
}
