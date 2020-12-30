#include "Timer.h"
#include <utility>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

dn::Timer::Timer(unsigned interval, TimerCallback callback):
	callback_(std::move(callback)),
	timerID_(0),
	interval_(interval)
{
	
}

void dn::Timer::setInterval(unsigned interval)
{
	interval_ = interval;
}

void dn::Timer::setCallback(TimerCallback callback)
{
	callback_ = std::move(callback);
}

void dn::Timer::start()
{
	if(timerID_)
		return;
	timerID_ = timeSetEvent(interval_, 1, reinterpret_cast<LPTIMECALLBACK>(timerProc), 
		reinterpret_cast<DWORD_PTR>(this), TIME_PERIODIC);
}

void dn::Timer::stop()
{
	if(timerID_)
	{
		timeKillEvent(timerID_);
		timerID_ = 0;
	}
}

void dn::Timer::timerProc(unsigned uID, unsigned uMsg, DWORD_PTR dwUser, DWORD dw1, DWORD dw2)
{
	auto *timer = reinterpret_cast<Timer*>(dwUser);
	if (timer->callback_)
		timer->callback_();
}

