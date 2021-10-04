#using <System.dll>
#include <Windows.h>
#include <conio.h>

#include <SMObject.h>
#include <smstructs.h>

using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;


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

	// LMS151 port number must be 2111
	int PortNumber = 2111;

	// Pointer to TcpClent type object on managed heap
	TcpClient^ Client;
	// arrays of unsigned chars to send and receive data
	array<unsigned char>^ SendData; // Managed Memory - Data to send 
	array<unsigned char>^ ReadData; // Managed Memory - Data to recieve
	// String command to ask for Channel 1 analogue voltage from the PLC
	// These command are available on Galil RIO47122 command reference manual
	// available online
	String^ AskScan = gcnew String("sRN LMDscandata");
	// String to store received data for display
	String^ ResponseData;

	// Creat TcpClient object and connect to it
	Client = gcnew TcpClient("192.168.5.8", PortNumber);
	// Configure connection - client settings
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;//ms
	Client->SendTimeout = 500;//ms
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;

	// unsigned char arrays of 16 bytes each are created on managed heap
	SendData = gcnew array<unsigned char>(16);
	ReadData = gcnew array<unsigned char>(2500);
	// Convert string command to an array of unsigned char
	SendData = System::Text::Encoding::ASCII->GetBytes(AskScan); // converting the ask scan string from ASCII into Bytes

	// Get the network streab object associated with clien so we 
	// can use it to read and write
	NetworkStream^ Stream = Client->GetStream();


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
		//Console::WriteLine("Laser time stamp    : {0,12:F3} {1,12:X2}", TimeStamp, Shutdown);

		// Write command asking for data
		Stream->WriteByte(0x02); // start transmission
		Stream->Write(SendData, 0, SendData->Length);
		Stream->WriteByte(0x03); // End Transmission
		// Wait for the server to prepare the data, 1 ms would be sufficient, but used 10 ms
		System::Threading::Thread::Sleep(10);
		// Read the incoming data
		Stream->Read(ReadData, 0, ReadData->Length);
		// Convert incoming data from an array of unsigned char bytes to an ASCII string
		ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
		// Print the received string on the screen
		Console::WriteLine(ResponseData);
		// Important fields required: pg 91 of data sheet
		// - Starting Angle -
		// - Angular step width
		// - NumberData
		// - Data_1 ... Data_n

		Thread::Sleep(25);
		
		if (_kbhit())
			break;
	}

	Stream->Close();
	Client->Close();

	// just to pause the actual thing
	Console::ReadKey();
	Console::ReadKey();


	return 0;
}