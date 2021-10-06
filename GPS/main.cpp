
#using <System.dll>
#include <Windows.h>
#include <conio.h>
#include "GPS.h" //include from the same directory you are in


#include <SMObject.h>
#include <smstructs.h>

using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;

int main()
{
	//Declaration
	SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SMObject GPSSMObject(TEXT("GPSSMObject"), sizeof(SM_GPS));
	//SM Creation and seeking access
	double TimeStamp;
	__int64 Frequency, Counter;
	int Shutdown = 0x00;

	QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);
	//PMObj.SMCreate();
	PMObj.SMAccess();
	//GPSSMObject.SMCreate();
	GPSSMObject.SMAccess();
	ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;
	SM_GPS* GPSData = (SM_GPS*)GPSSMObject.pData;
	PMData->Shutdown.Flags.GPS = 0;

	while (!PMData->Shutdown.Flags.GPS){
		PMData->Heartbeat.Flags.GPS = 1; // Set heartbeat flag

		if (PMData->PMHeartbeat.Flags.GPS == 1) {
			PMData->PMHeartbeat.Flags.GPS = 0;
			PMData->PMCounter[3] = 0;
		}
		else {
			Console::WriteLine("PM Counter: {0:D}", PMData->PMCounter[3]);
			PMData->PMCounter[3]++;
			if  (PMData->PMCounter[3] > PM_WAIT) {
				//PMData->Shutdown.Status = 0xFF;
				break;
			}
		}

		QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
		TimeStamp = (double)Counter / (double)Frequency * 1000; // ms
		Console::WriteLine("GPS time stamp  : {0,12:F3} {1,12:X2}", TimeStamp, Shutdown);
		Thread::Sleep(50);
		if (_kbhit())
			break;
	}

	Console::ReadKey();
	Console::ReadKey();

	return 0;
}