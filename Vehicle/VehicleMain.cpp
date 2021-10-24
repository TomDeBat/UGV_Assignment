#using <System.dll>
#include <Windows.h>
#include <conio.h>
#include "VehicleControl.h" //include from the same directory you are in

#include <SMObject.h>
#include <smstructs.h>

using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;

int main()
{
	VehicleControl VehicleClass;

	//Sleep(100);
	VehicleClass.setupSharedMemory();

	int PortNumber = 25000;
	String^ HostName = "5209309\n";
	VehicleClass.connect(HostName, PortNumber);

	while (!VehicleClass.getShutdownFlag()) {
		bool temp = true;
		if (_kbhit() || VehicleClass.setHeartbeat(temp) == 0) {
			break;
		}
		VehicleClass.getData();
		VehicleClass.sendDataToSharedMemory();
	}

	VehicleClass.~VehicleControl();

	//Console::ReadKey();
	//Console::ReadKey();


	return 0;
}