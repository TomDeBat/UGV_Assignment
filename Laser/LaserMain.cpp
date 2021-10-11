#using <System.dll>
#include <Windows.h>
#include <conio.h>
#include <math.h>
#include <UGV_module.h>

#include <SMObject.h>
#include <smstructs.h>
#include "Laser.h"

using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

int main()
{
	Laser LaserClass;

	
	LaserClass.setupSharedMemory();

	int PortNumber = 23000;
	String^ HostName = "z5209309\n";
	//LaserClass.connect(HostName, PortNumber);

	while(!LaserClass.getShutdownFlag()){
		bool temp = true;
		if (_kbhit() || LaserClass.setHeartbeat(temp) == 0) {
			break;
		}
		//LaserClass.getData();
		//LaserClass.checkData();
		//LaserClass.sendDataToSharedMemory();
	}

	//Console::ReadKey();
	//Console::ReadKey();



	return 0;
}