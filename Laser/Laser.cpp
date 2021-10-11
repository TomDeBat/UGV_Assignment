
#include "laser.h"
#using <System.dll>
#include <Windows.h>
#include <conio.h>
#include <math.h>

#include <SMObject.h>
#include <smstructs.h>

using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;


int Laser::connect(String^ hostName, int portNumber)
{
	AskScan = gcnew String("sRN LMDscandata");
	StudID = gcnew String("5209309\n");
	// String to store received data for display
	Console::WriteLine("Hello THere");

	// Creat TcpClient object and connect to it
	Client = gcnew TcpClient("192.168.1.200", portNumber);
	// Configure connection - client settings
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;//ms
	Client->SendTimeout = 500;//ms
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;

	// unsigned char arrays of 16 bytes each are created on managed heap
	SendData = gcnew array<unsigned char>(16);
	ReadData = gcnew array<unsigned char>(2500);



	// Get the network streab object associated with clien so we 
	// can use it to read and write
	Stream = Client->GetStream(); // means of transmitting

		// Convert string command to an array of unsigned char
	SendData = System::Text::Encoding::ASCII->GetBytes(StudID); // converting the ask scan string from ASCII into Bytes

	// Authenticate user
	Stream->Write(SendData, 0, SendData->Length);
	// Wait for the server to prepare the data, 1 ms would be sufficient, but used 10 ms
	System::Threading::Thread::Sleep(10);
	// Read the incoming data
	Stream->Read(ReadData, 0, ReadData->Length);
	ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
	Console::WriteLine(ResponseData);
	//Console::ReadKey();
	SendData = System::Text::Encoding::ASCII->GetBytes(AskScan);

	// YOUR CODE HERE
	return 1;
}
int Laser::setupSharedMemory()
{
	ProcessManagementData = new SMObject(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SensorData = new SMObject(_TEXT("LaserSMObject"), sizeof(SM_Laser));
	//ProcessManagementData->SMCreate();
	//SensorData->SMCreate();
	ProcessManagementData->SMAccess();
	SensorData->SMAccess();
	PMData =(ProcessManagement*)ProcessManagementData->pData;
	LaserData = (SM_Laser*)SensorData->pData;
	PMData->Shutdown.Flags.Laser = 0;

	return 1;
}
int Laser::getData()
{
	Stream->WriteByte(0x02); // start transmission
	Stream->Write(SendData, 0, SendData->Length);
	Stream->WriteByte(0x03); // End Transmission
	// Wait for the server to prepare the data, 1 ms would be sufficient, but used 10 ms
	System::Threading::Thread::Sleep(100);
	// Read the incoming data
	Stream->Read(ReadData, 0, ReadData->Length);
	// Convert incoming data from an array of unsigned char bytes to an ASCII string
	ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
	// Print the received string on the screen
	//Console::WriteLine(ResponseData);
	// YOUR CODE HERE
	return 1;
}
int Laser::checkData()
{
	array<String^>^ LaserData = ResponseData->Split(' ');
	Console::WriteLine(LaserData[1]);
	Console::WriteLine(LaserData[20]);
	double StartAngle = System::Convert::ToInt32(LaserData[23], 16);
	double AngularStep = System::Convert::ToInt32(LaserData[24], 16) / 10000.0;
	double NumberData = System::Convert::ToInt32(LaserData[25], 16);
	Console::WriteLine("The start angle: {0, 0:F0}", StartAngle);
	Console::WriteLine("The step size: {0, 0:F3}", AngularStep);
	Console::WriteLine("The number of data  points: {0, 0:F0}", NumberData);



	XRange = gcnew array<double>(NumberData);
	YRange = gcnew array<double>(NumberData);
	double temp, angle;

	for (int i = 0; i < NumberData; i++) {
		temp = System::Convert::ToInt32(LaserData[26 + i], 16);
		XRange[i] = temp * cos(i * AngularStep * PI_DEF / 180);
		YRange[i] = temp * sin(i * AngularStep * PI_DEF / 180);
		Console::WriteLine("x:{0, 0:F4} y:{1, 0:F4}", XRange[i], YRange[i]);
	}

	// YOUR CODE HERE
	return 1;
}
int Laser::sendDataToSharedMemory()
{

	for (int i = 0; i < NumberData; i++) {
		LaserData->x[i] = XRange[i];
		LaserData->y[i] = YRange[i];
	}
	// YOUR CODE HERE
	return 1;
}
bool Laser::getShutdownFlag()
{
	temp = false;
	if (PMData->Shutdown.Flags.Laser == 1) {
		temp = true;
	}
	return temp;
}
int Laser::setHeartbeat(bool heartbeat)
{

	Thread::Sleep(25);
	PMData->Heartbeat.Flags.Laser = 1; // Set heartbeat flag
	//PMData->PMHeartbeat.Flags.Laser = 1; // JUST FOR TESTING

	if (PMData->PMHeartbeat.Flags.Laser == 1) {
		PMData->PMHeartbeat.Flags.Laser = 0;
		PMData->PMCounter[LASER_POS] = 0;
	}
	else {
		PMData->PMCounter[LASER_POS]++;
		Console::WriteLine("PM Counter: {0:D}", PMData->PMCounter[LASER_POS]);
		if (PMData->PMCounter[LASER_POS] > PM_WAIT) {
			PMData->Shutdown.Status = 0xFF;
			PMData->PMCounter[LASER_POS] = 0;
			return 0;
		}
	}
	return 1;
}
Laser::~Laser()
{
	// YOUR CODE HERE
	Stream->Close();
	Client->Close();
}
