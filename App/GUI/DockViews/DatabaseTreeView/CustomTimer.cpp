#include "stdafx.h"
#include "CustomTimer.h"

#pragma comment(lib, "Winmm.lib")

// Function to set a timer
void Ctimer::MySetTimer(UINT uLen)
{
	MMRESULT TimerID;
	LPTIMECALLBACK lpTimeProc;

	m_ti.pseudoThis = (DWORD) this;
	m_ti.dwInstance = uLen;

	lpTimeProc = &MyTimerCallBack;

	TimerID = timeSetEvent(
		uLen,
		uLen / 2,
		lpTimeProc,
		(DWORD)&m_ti,
		TIME_PERIODIC);

	m_id = (UINT)TimerID;
}

// Tick event handler
// Called by callback wrapper
void Ctimer::TimerTick(DWORD dwMyData)
{
	TRACE("Tick: %d ms", dwMyData);
}

// Callback function
// Unbundles thisInstance and
// calls member function
void CALLBACK MyTimerCallBack(UINT uTimerID, UINT uMsg,	DWORD dwUser, DWORD dw1,DWORD dw2)
{
	LPTHISINSTANCE lpti = NULL;
	Ctimer  *pseudoThis = NULL;
	DWORD    dwData;

	// get thisinstance struct
	lpti = (LPTHISINSTANCE)dwUser;

	// get pseudo this from struct
	pseudoThis = (Ctimer *)lpti->pseudoThis;

	// Get other instance data
	dwData = lpti->dwInstance;

	//call bound member function
	pseudoThis->TimerTick(dwData);
}
