#pragma once
#include "stdafx.h"

class GetterExit
{
	std::unique_ptr<std::thread> workerThread{};
	std::atomic<bool> m_exitState{ false };
public:
	GetterExit() { startThread(); }
	~GetterExit() { stopThread(); }
	//Returns a bool indicating if the thread should stop.
	bool operator()() { return m_exitState; }
protected:
	void stopThread() const
	{
		if (workerThread)
		{
			if (workerThread->joinable())
			{
				workerThread->join();
			}
		}
	}
	void startThread()
	{
		workerThread = std::make_unique<std::thread>([this]() { workThread(); });
	}
	void workThread()
	{
		std::cin.get(); // block and wait for enter key
		std::cin.clear();
		m_exitState = true;
	}
};