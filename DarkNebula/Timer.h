#pragma once
#include <functional>
#include <Windows.h>
#include "DarkNebulaGlobal.h"
namespace dn
{
	typedef std::function<void(void)> TimerCallback;
	
	// 高精度定时器类，使用windows多媒体定时器实现，只实现循环定时器。
	class DN_EXPORT Timer
	{
	public:
		
		Timer(unsigned interval = 1, TimerCallback callback = nullptr);

		// ms
		void setInterval(unsigned interval);

		void setCallback(TimerCallback callback);

		void start();

		void stop();

		bool isRunning() const;

	private:
		static void __stdcall timerProc(unsigned uID, unsigned uMsg, DWORD_PTR dwUser, DWORD dw1, DWORD dw2);
		
		TimerCallback callback_;
		unsigned timerID_;
		unsigned interval_;
	};
}
