#ifndef CELLTimestamp_hpp
#define CELLTimestamp_hpp
#include <chrono>

using namespace std::chrono;
class CELLTimestamp
{
public:
	CELLTimestamp()
	{
		update();
	}
	~CELLTimestamp()
	{}

	void update()
	{
		_begin = high_resolution_clock::now();
	}


	//��ȡ��ǰ��
	double getElapsedSecond()
	{
		return getElapsedTimeInMicroSec() * 0.000001;
	}
	//��ȡ����
	double getElapsedTimeInMilliSec()
	{
		return this->getElapsedTimeInMicroSec() * 0.001;
	}

	//��ȡ΢��
	long long getElapsedTimeInMicroSec()
	{
		return duration_cast<microseconds>(high_resolution_clock::now() - _begin).count();
	}

protected:
	time_point<high_resolution_clock>_begin;
};
#endif
/*
#pragma once
#include <Windows.h>

class CELLTimestamp
{
public:
	CELLTimestamp()
	{
		QueryPerformanceFrequency(&_frequency);
		QueryPerformanceCounter(&_startCount);
	}
	~CELLTimestamp()
	{}

	void update()
	{
		QueryPerformanceCounter(&_startCount);
	}

	
	//��ȡ��ǰ��
	double getElapsedSecond()
	{
		return getElapsedTimeInMicroSec() * 0.000001;
	}
	//��ȡ����
	double getElapsedTimeInMilliSec()
	{
		return this->getElapsedTimeInMicroSec() * 0.001;
	}

	//��ȡ΢��
	double getElapsedTimeInMicroSec()
	{
		LARGE_INTEGER endCount;
		QueryPerformanceCounter(&endCount);

		double startTimeInMicroSec = _startCount.QuadPart * (1000000.0 / _frequency.QuadPart);
		double endTimeInMicroSec = endCount.QuadPart * (1000000.0 / _frequency.QuadPart);

		return endTimeInMicroSec - startTimeInMicroSec;
	}

protected:
	LARGE_INTEGER _frequency;
	LARGE_INTEGER _startCount;
};
*/