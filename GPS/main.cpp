
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
	GPS GPSClass;

	//Sleep(100);
	GPSClass.setupSharedMemory();

	int PortNumber = 24000;
	String^ HostName = "COM1";
	GPSClass.connect(HostName, PortNumber);

	while (!GPSClass.getShutdownFlag()) {
		bool temp = true;
		if (_kbhit() || GPSClass.setHeartbeat(temp) == 0) {
			break;
		}
		GPSClass.getData();
		
	}

	GPSClass.~GPS();

	//Console::ReadKey();
	//Console::ReadKey();

	return 0;
}