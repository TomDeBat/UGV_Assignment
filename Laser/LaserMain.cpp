#using <System.dll>
#include <Windows.h>
#include <conio.h>

#include <SMObject.h>
#include <smstructs.h>

using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;

int main()
{
	//Declaration
	SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SMObject LaserSMObject(TEXT("LaserSMObject"), sizeof(SM_Laser));

	//SM Creation and seeking access
	double TimeStamp;
	__int64 Frequency, Counter;
	int Shutdown = 0x00;
	QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);

	PMObj.SMAccess();
	LaserSMObject.SMAccess();

	ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;
	SM_Laser* LaserData = (SM_Laser*)LaserSMObject.pData;
	PMData->Shutdown.Flags.Laser = 0;

	while (!PMData->Shutdown.Flags.Laser)
	{
		PMData->Heartbeat.Flags.Laser = 1; // Set heartbeat flag

		if (PMData->PMHeartbeat.Flags.Laser == 1) {
			PMData->PMHeartbeat.Flags.Laser = 0;
			PMData->PMCounter[0] = 0;
		}
		else {
			PMData->PMCounter[0]++;
			Console::Write("PM Counter: {0:D}", PMData->PMCounter[0]);
			if (PMData->PMCounter[0] > PM_WAIT) {
				PMData->Shutdown.Status = 0xFF;
				PMData->PMCounter[0] = 0;
				break;
			}
		}
		QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
		TimeStamp = (double)Counter / (double)Frequency * 1000; // ms
		Console::WriteLine("Laser time stamp    : {0,12:F3} {1,12:X2}", TimeStamp, Shutdown);
		Thread::Sleep(25);
		
		if (_kbhit())
			break;
	}


	return 0;
}