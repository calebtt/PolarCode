#include "stdafx.h"
#include "PolarCalc.h"
#include "GetterExit.h"
static constexpr auto newl = '\n';

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

	cout << "Radius: " << polarRadius << newl;
	cout << "Theta Angle: " << polarTheta << " Half Angle is: " << ((rangePair.second - rangePair.first) / 2.0) + rangePair.first << newl;
	cout << "X adjusted mag: " << xMagnitude << newl;
	cout << "Y adjusted mag: " << yMagnitude << newl;
	cout << "Quadrant: " << quadResult << " With bounds: " << std::get<0>(rangePair) << "," << std::get<1>(rangePair) << newl;
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
			DoPolarCalc(state);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(4));
	}
}