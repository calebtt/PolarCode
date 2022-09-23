#include "stdafx.h"
#include "PolarCalc.h"
#include "GetterExit.h"
static constexpr auto newl = '\n';


struct MouseSettings
{
	//PolarRadiusValueMax is the sentinel abs value of a computed polar radius.
	static constexpr int PolarRadiusValueMax{ 39'000 };
	//Microseconds Min is the minimum delay for the thumbstick axis thread loop at the highest thumbstick value.
	static constexpr int MICROSECONDS_MIN{ 1100 };
	//Microseconds Max is the maximum delay for the thumbstick axis thread loop at the lowest thumbstick value.
	static constexpr int MICROSECONDS_MAX{ 24'000 };
};
MouseSettings m_mouse_settings{};

[[nodiscard]] auto ConvertMagnitudesToDelays(const auto xPolarDelay, const auto yPolarDelay) -> std::pair<size_t, size_t>
{
	const double xPercentOfRadius = xPolarDelay / static_cast<double>(m_mouse_settings.PolarRadiusValueMax);
	const double yPercentOfRadius = yPolarDelay / static_cast<double>(m_mouse_settings.PolarRadiusValueMax);
	auto xResult = static_cast<size_t>(std::lerp(m_mouse_settings.MICROSECONDS_MIN, m_mouse_settings.MICROSECONDS_MAX, xPercentOfRadius));
	auto yResult = static_cast<size_t>(std::lerp(m_mouse_settings.MICROSECONDS_MIN, m_mouse_settings.MICROSECONDS_MAX, yPercentOfRadius));
	return { xResult, yResult };
}

void DoPolarCalc(const XINPUT_STATE &state)
{
	using std::cout;
	auto LogFn = [](const char* str) { std::cerr << str << newl; };
	sds::PolarCalc pc(32'766, LogFn);
	//get info members
	const auto polarComplete = pc.ComputePolarCompleteInfo(state.Gamepad.sThumbRX, state.Gamepad.sThumbRY);
	//alias for adjusted magnitude values
	const auto [xMagnitude, yMagnitude] = polarComplete.adjusted_magnitudes;
	//alias for polar radius and polar theta angle
	const auto [polarRadius, polarTheta] = polarComplete.polar_info;
	//alias for quadrant range pair and quadrant number
	const auto [rangePair, quadResult] = polarComplete.quadrant_info;
	//convert magnitudes to delays (directly)
	const auto [xDelay, yDelay] = ConvertMagnitudesToDelays(xMagnitude, yMagnitude);
	cout << "Radius: " << polarRadius << newl;
	cout << "Theta Angle: " << polarTheta << " Half Angle is: " << ((rangePair.second - rangePair.first) / 2.0) + rangePair.first << newl;
	cout << "X adjusted mag: " << xMagnitude << newl;
	cout << "Y adjusted mag: " << yMagnitude << newl;
	cout << "Quadrant: " << quadResult << " With bounds: " << std::get<0>(rangePair) << "," << std::get<1>(rangePair) << newl;
	cout << "-------" << newl;
	cout << "Xdelay:" << xDelay << newl;
	cout << "Ydelay:" << yDelay << newl;
	cout << "-------" << newl;
	cout << "Cos x:" << std::cos(polarTheta) * polarRadius << newl;
	cout << "Sin y:" << std::sin(polarTheta) * polarRadius << newl;
	cout << "-------" << newl;
}

void DoScaleInputValues(const XINPUT_STATE& state)
{
	using std::cout;
	auto LogFn = [](const char* str) { std::cerr << str << newl; };
	sds::PolarCalc pc(32'766, LogFn);
	//get info members
	const auto polarComplete = pc.ComputePolarCompleteInfo(state.Gamepad.sThumbRX, state.Gamepad.sThumbRY);
	//alias for adjusted magnitude values
	const auto [xMagnitude, yMagnitude] = polarComplete.adjusted_magnitudes;
	//alias for polar radius and polar theta angle
	const auto [polarRadius, polarTheta] = polarComplete.polar_info;
	//alias for quadrant range pair and quadrant number
	const auto [rangePair, quadResult] = polarComplete.quadrant_info;
	//convert magnitudes to delays (directly)
	const auto [xDelay, yDelay] = ConvertMagnitudesToDelays(xMagnitude, yMagnitude);
	cout << "Hardware: [X]:" << state.Gamepad.sThumbRX << " [Y]:" << state.Gamepad.sThumbRY << newl;
	cout << "-------" << newl;
	cout << "Radius: " << polarRadius << newl;
	cout << "Theta Angle: " << polarTheta << " Half Angle is: " << ((rangePair.second - rangePair.first) / 2.0) + rangePair.first << newl;
	cout << "X adjusted mag: " << xMagnitude << newl;
	cout << "Y adjusted mag: " << yMagnitude << newl;
	cout << "Quadrant: " << quadResult << " With bounds: " << std::get<0>(rangePair) << "," << std::get<1>(rangePair) << newl;
	cout << "-------" << newl;
	cout << "Xdelay:" << xDelay << newl;
	cout << "Ydelay:" << yDelay << newl;
	cout << "-------" << newl;
	cout << "Cos x:" << std::cos(polarTheta) * polarRadius << newl;
	cout << "Sin y:" << std::sin(polarTheta) * polarRadius << newl;
	cout << "-------" << newl;
}

int main(int argc, const char* argv[])
{
	using std::wstring;
	using std::wcout;
	using std::cout;
	using std::endl;
	GetterExit g{};

	//test output
	while (!g())
	{
		XINPUT_STATE state{};
		const auto res = XInputGetState(0, &state);
		if (res == ERROR_SUCCESS)
		{
			//DoPolarCalc(state);
			DoScaleInputValues(state);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(4));
	}
}