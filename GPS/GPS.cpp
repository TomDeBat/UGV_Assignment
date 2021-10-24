#include "GPS.h"
#using <System.dll>
#include <Windows.h>
#include <conio.h>
#include <math.h>

#include <SMObject.h>
#include <smstructs.h>

using namespace System;
using namespace System::IO::Ports;
using namespace System::Diagnostics;
using namespace System::Threading;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

struct GPSDataStruct;




int GPS::connect(String^ hostName, int portNumber)
{
	

	SendData = gcnew array<unsigned char>(16);
	ReadData = gcnew array<unsigned char>(224);

	//Port = gcnew SerialPort;
	//PortName = gcnew String(hostName);

	//Port->PortName = PortName;
	//Port->BaudRate = 115200;
	//Port->StopBits = StopBits::One;
	//Port->DataBits = 8;
	//Port->Parity = Parity::None;
	//Port->Handshake = Handshake::None;

	//// Set the read/write timeouts & buffer size
	//Port->ReadTimeout = 500;
	//Port->WriteTimeout = 500;
	//Port->ReadBufferSize = 224;
	//Port->WriteBufferSize = 1024;

	//Port->Open();
	//Port->Read(ReadData, 0, sizeof(GPSDataStruct));

	//ReadData = gcnew array<unsigned char>(112) { 0xaa, 0x44, 0x12, 0x1c, 0xd6, 0x02, 0x02, 0x20, 0x50, 0x00, 0x00, 0x00, 0x64, 0xb4, 0x94, 0x05, 0xf6, 0xc4, 0x39, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x8c, 0xef, 0x81, 0x08, 0x00, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x76, 0xdf, 0xb9, 0x9e, 0xb3, 0xc8, 0x57, 0x41, 0xfd, 0xbb, 0x6c, 0xcd, 0xb4, 0x5a, 0x13, 0x41, 0x00, 0x00, 0x60, 0x07, 0xe8, 0x18, 0x5b, 0x40, 0x81, 0x7c, 0xa5, 0x41, 0x3d, 0x00, 0x00, 0x00, 0x07, 0xb1, 0x8a, 0x3c, 0xf4, 0x39, 0x03, 0x3d, 0x4c, 0xd7, 0x30, 0x3d, 0x41, 0x41, 0x41, 0x41, 0xcd, 0xcc, 0xac, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x09, 0x07, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x04, 0xa3, 0xfd, 0xcc };

	// does not require authentication like Laser
	Client = gcnew TcpClient("192.168.1.200", portNumber);
	// Configure connection
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;//ms
	Client->SendTimeout = 500;//ms
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;


	Stream = Client->GetStream();
	Console::Write(Stream->DataAvailable);
	// YOUR CODE HERE
	return 1;
}
int GPS::setupSharedMemory() 
{
	ProcessManagementData = new SMObject(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SensorData = new SMObject(_TEXT("GPSSMObject"), sizeof(SM_GPS));
	//ProcessManagementData->SMCreate();
	//SensorData->SMCreate();
	ProcessManagementData->SMAccess();
	SensorData->SMAccess();
	PMData = (ProcessManagement*)ProcessManagementData->pData;
	GPSData = (SM_GPS*)SensorData->pData;
	PMData->Shutdown.Flags.GPS = 0;
	return 1;
}
int GPS::getData() 
{	
	//GPSDataStruct* Novatel = new GPSDataStruct;
	GPSDataStruct Novatel;
	BytePtr = (unsigned char*)&Novatel;
	startBytePtr = BytePtr;
	int debugging = 1;
	
	//Stream->DataAvailable <- put this back in for main thing
	if (Stream->DataAvailable) {
		Stream->Read(ReadData, 0, ReadData->Length);
		//
		Start = checkData();
		for (int i = Start; i < Start + sizeof(Novatel); i++) {
			*(BytePtr++) = ReadData[i];

		}

		unsigned long temp = CalculateBlockCRC32(sizeof(GPSDataStruct) - 4, startBytePtr);
		if (Novatel.CRC == temp) {
			tempEasting = Novatel.Easting;
			tempNorthing = Novatel.Northing;
			tempHeight = Novatel.Height;
			Console::WriteLine("Northing: {0,10:F3}  Easting: {1,10:F3}  Height: {2,10:F3} CRC: {3, 10:X}", Novatel.Northing,
				Novatel.Easting, Novatel.Height, temp);
			sendDataToSharedMemory();


		}
	}
	// YOUR CODE HERE
	return 1;
}

int GPS::checkData() 
{
	// run this first before getData - checks when valid data stream occurs
	// YOUR CODE HERE
	unsigned int Header = 0;
	int i = 0;
	unsigned char Data;

	do
	{
		Data = ReadData[i++];
		Console::WriteLine("Data Output: {0,5:F3}", Data);
		Header = ((Header << 8) | Data);
	} while (Header != 0xaa44121c);

	return i - 4;
}

int GPS::sendDataToSharedMemory() 
{
	GPSData->easting = tempEasting;
	GPSData->northing = tempNorthing;
	GPSData->height = tempHeight;
	// YOUR CODE HERE
	return 1;
}

bool GPS::getShutdownFlag() 
{
	bool temp = false;
	if (PMData->Shutdown.Flags.GPS == 1) {
		temp = true;
	}
	return temp;
}

int GPS::setHeartbeat(bool heartbeat) 
{
	PMData->Heartbeat.Flags.GPS = 1; // Set heartbeat flag
	//PMData->PMHeartbeat.Flags.GPS = 1; // FOR DEBUGGING
	Sleep(100);
	if (PMData->PMHeartbeat.Flags.GPS == 1) {
		PMData->PMHeartbeat.Flags.GPS = 0;
		PMData->PMCounter[GPS_POS] = 0;
	}
	else {
		Console::WriteLine("PM Counter: {0:D}", PMData->PMCounter[GPS_POS]);
		PMData->PMCounter[GPS_POS]++;
		if (PMData->PMCounter[GPS_POS] > PM_WAIT) {
			PMData->Shutdown.Status = 0xFF;
			return 0;
		}
	}
	return 1;
}

GPS::~GPS()
{
	// YOUR CODE HERE
	Stream->Close();
	Client->Close();

}

// verify the GPS data has been recieved by the code directly
unsigned long CRC32Value(int i)
{
	int j;
	unsigned long ulCRC;
	ulCRC = i;
	for (j = 8; j > 0; j--)
	{
		if (ulCRC & 1)
			ulCRC = (ulCRC >> 1) ^ CRC32_POLYNOMIAL;
		else
			ulCRC >>= 1;
	}
	return ulCRC;
}

unsigned long CalculateBlockCRC32(unsigned long ulCount, /* Number of bytes in the data block */
	unsigned char* ucBuffer) /* Data block */
{
	unsigned long ulTemp1;
	unsigned long ulTemp2;
	unsigned long ulCRC = 0;
	while (ulCount-- != 0)
	{
		ulTemp1 = (ulCRC >> 8) & 0x00FFFFFFL;
		ulTemp2 = CRC32Value(((int)ulCRC ^ *ucBuffer++) & 0xff);
		ulCRC = ulTemp1 ^ ulTemp2;
	}
	return(ulCRC);
}
