#include "VehicleControl.h"
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


int VehicleControl::connect(String^ hostName, int portNumber)
{
	//AskScan = gcnew String("sRN LMDscandata");
	StudID = gcnew String("5209309\n");
	// String to store received data for display
	//Console::WriteLine("Hello THere");

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



	// Get the network streab object associated with client so we 
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
	//SendData = System::Text::Encoding::ASCII->GetBytes(AskScan);

	return 1;
}
int VehicleControl::setupSharedMemory()
{
	ProcessManagementData = new SMObject(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SensorData = new SMObject(_TEXT("VehicleSMObject"), sizeof(SM_VehicleControl));
	//ProcessManagementData->SMCreate();
	//SensorData->SMCreate();
	ProcessManagementData->SMAccess();
	SensorData->SMAccess();
	PMData = (ProcessManagement*)ProcessManagementData->pData;
	VehicleControl = (SM_VehicleControl*)SensorData->pData;
	PMData->Shutdown.Flags.VehicleControl = 0;

	return 1;
}
int VehicleControl::getData()
{
	VehicleInput = gcnew String("# " + VehicleControl->Steering.ToString("F3") + " " + VehicleControl->Speed.ToString("F3") + " " + flag + " #");
	flag = !flag;
	Console::WriteLine(VehicleInput);
	// YOUR CODE HERE
	return 1;
}
int VehicleControl::checkData()
{

	// YOUR CODE HERE
	return 1;
}
int VehicleControl::sendDataToSharedMemory()
{
	// YOUR CODE HERE
	SendData = System::Text::Encoding::ASCII->GetBytes(VehicleInput);
	Stream->WriteByte(0x02);
	Stream->Write(SendData, 0, SendData->Length);
	Stream->WriteByte(0x03);
	System::Threading::Thread::Sleep(30);
	return 1;
}
bool VehicleControl::getShutdownFlag()
{
	bool temp = false;
	if (PMData->Shutdown.Flags.VehicleControl == 1) {
		temp = true;
	}
	return temp;
}
int VehicleControl::setHeartbeat(bool heartbeat)
{
	PMData->Heartbeat.Flags.VehicleControl = 1;
	if (PMData->PMHeartbeat.Flags.VehicleControl == 1) {
		PMData->PMHeartbeat.Flags.VehicleControl = 0;
		PMData->PMCounter[VEHICLE_POS] = 0;
	}
	else {
		PMData->PMCounter[VEHICLE_POS]++;
		Console::WriteLine("PM Counter: {0:D}", PMData->PMCounter[VEHICLE_POS]);
		if (PMData->PMCounter[VEHICLE_POS] > PM_WAIT) {
			PMData->Shutdown.Status = 0xFF;
			PMData->PMCounter[VEHICLE_POS] = 0;
			return 0;
		}
	}
	return 1;
}
VehicleControl::~VehicleControl()
{
	// YOUR CODE HERE
	Stream->Close();
	Client->Close();
}