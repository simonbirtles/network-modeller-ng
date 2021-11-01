#pragma once

#include "stdafx.h"
#include "mmsystem.h"

// Callback function prototype
// (from WIN32 SDK)
extern "C"
{
	void CALLBACK MyTimerCallBack(UINT uTimerID,UINT uMsg,DWORD dwUser,	DWORD dw1,DWORD dw2);
}

// pseudo-this bundle structure
typedef struct tagthisInstance
{
	DWORD   pseudoThis;
	DWORD   dwInstance;
} thisInstance, *LPTHISINSTANCE;


// Timer class
class Ctimer
{
public:
	Ctimer() {}
	~Ctimer()
	{
		timeKillEvent(m_id);
	}
	void MySetTimer(UINT uLen);
	void TimerTick(DWORD dwMyData);
private:
	thisInstance   m_ti;
	UINT           m_id;
};


//End of File
