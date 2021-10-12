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
	SMObject VehicleSMObject(TEXT("VehicleSMObject"), sizeof(SM_VehicleControl));
	//SM Creation and seeking access
	double TimeStamp;
	__int64 Frequency, Counter;
	int Shutdown = 0x00;

	QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);
	PMObj.SMAccess();
	VehicleSMObject.SMAccess();
	ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;

	PMData->Shutdown.Flags.VehicleControl = 0;

	while (!PMData->Shutdown.Flags.VehicleControl)
	{
		PMData->Heartbeat.Flags.VehicleControl = 1; // Set heartbeat flag

		if (PMData->PMHeartbeat.Flags.VehicleControl == 1) {
			PMData->PMHeartbeat.Flags.VehicleControl = 0;
			PMData->PMCounter[VEHICLE_POS] = 0;
		}
		else {
			PMData->PMCounter[VEHICLE_POS]++;
			Console::Write("PM Counter: {0:D}", PMData->PMCounter[VEHICLE_POS]);
			if (PMData->PMCounter[VEHICLE_POS] > PM_WAIT) {
				PMData->Shutdown.Status = 0xFF;
				PMData->PMCounter[VEHICLE_POS] = 0;
				break;
			}
		}
		QueryPerformanceCounter((LARGE_INTEGER*)&Counter);
		TimeStamp = (double)Counter / (double)Frequency * 1000000; // ms
		Console::WriteLine("VehicleControl time stamp    : {0,12:F3} {1,12:X2}", TimeStamp, Shutdown);
		Thread::Sleep(25);

		if (_kbhit())
			break;
	}

	return 0;
}