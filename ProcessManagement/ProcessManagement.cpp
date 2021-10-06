
#using <System.dll>
#include <Windows.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <iostream>
#include <conio.h>

//paste all the SMObject into the source files folder for each of the sections
#include <SMObject.h>
#include <smstructs.h>	



using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;


TCHAR Units[10][20] = //
{
	TEXT("Laser.exe"),
	TEXT("Display.exe"),
	TEXT("Vehicle.exe"),
	TEXT("GPS.exe"),
	TEXT("Camera.exe")
};


int main() {

	// tele-operation
		// Declaration
	SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SMObject LaserSMObject(TEXT("LaserSMObject"), sizeof(SM_Laser));
	SMObject GPSSMObject(TEXT("GPSSMObject"), sizeof(SM_GPS));
	SMObject VehicleSMObject(TEXT("VehicleSMObject"), sizeof(SM_VehicleControl));


	array<String^>^ Modulelist = gcnew array<String^>{"Laser", "Display", "Vehicle", "GPS", "Camera"};
	array<int>^ Critical = gcnew array<int>(Modulelist->Length) { 1, 1, 1, 0, 0 };
	array<Process^>^ ProcessList = gcnew array<Process^>(Modulelist->Length);

	//SM Creation and Seeking access
	PMObj.SMCreate();
	PMObj.SMAccess();
	LaserSMObject.SMCreate();
	LaserSMObject.SMAccess();
	GPSSMObject.SMCreate();
	GPSSMObject.SMAccess();
	VehicleSMObject.SMCreate();
	VehicleSMObject.SMAccess();


	ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;
	SM_Laser* LaserData = (SM_Laser*)LaserSMObject.pData;
	SM_GPS* GPSData = (SM_GPS*)GPSSMObject.pData;
	SM_VehicleControl* VehicleData = (SM_VehicleControl*)VehicleSMObject.pData;

	PMData->PMHeartbeat.Status = 0xFF;
	//Modulelist->Length
	for (int i = 1; i < 2; i++) {
		//creates an array of all information processes
		if (Process::GetProcessesByName(Modulelist[i])->Length == 0) {
			ProcessList[i] = gcnew Process;
			ProcessList[i]->StartInfo->FileName = Modulelist[i];
			Console::WriteLine("The process " + Modulelist[i]);
			ProcessList[i]->StartInfo->WorkingDirectory = "../Executable";
			//ProcessList[i]->Kill();
			ProcessList[i]->Start();
			Console::WriteLine("The process " + Modulelist[i] + ".exe started");
			Thread::Sleep(100);

		}
	}
	// need to deal with priorities

	//Main Loop
	int LaserCounter = 0;
	int DisplayCounter = 0;
	int GPSCounter = 0;
	int VehicleCounter = 0;
	int CameraCounter = 0;

	while (!_kbhit()) {

		
		PMData->PMHeartbeat.Status = 0xFF;
		
		/*
		// Laser - critical
		if (PMData->Heartbeat.Flags.Laser == 1) {
			LaserCounter = 0;
			Console::WriteLine("Laser is Alive");
			PMData->Heartbeat.Flags.Laser = 0;
		} else {
			if (LaserCounter < LASER_WAIT) {
				LaserCounter++;
			} else {
				Console::WriteLine("The process" + Modulelist[0] + " is critical, total shutdown required");
				//PMData->Shutdown.Status = 0xFF;
				//break;
			}


		}
		*/
		// Display - non critical
		if (PMData->Heartbeat.Flags.Display == 1) {
			DisplayCounter = 0;
			PMData->Heartbeat.Flags.Display = 0;
			Console::WriteLine("Display is Alive");
		}
		else {
			// checks if wait time has elapsed
			Console::WriteLine("Display  wait is " + DisplayCounter);
			if (DisplayCounter < DISPLAY_WAIT) {
				DisplayCounter++;
			}
			else {
				// checks if program is running
				if (ProcessList[1]->HasExited) {
					//if its not running start the program
					Console::WriteLine("The process" + Modulelist[1] + " has exited, starting up");
					ProcessList[1]->Start();
					DisplayCounter = 0;
				}
				else {
					// Kill process then start running it again
					Console::WriteLine("The process" + Modulelist[1] + " is being restarted");
					ProcessList[1]->Kill();
					ProcessList[1]->Start();
					DisplayCounter = 0;
				}
			}

		}
		/*
		// Vehicle Control - critical
		if (PMData->Heartbeat.Flags.VehicleControl == 1) {
			VehicleCounter = 0;
			PMData->Heartbeat.Flags.VehicleControl = 0;
		}
		else {
			if (VehicleCounter < VEHICLE_WAIT) {
				VehicleCounter++;
			}
			else {
				Console::WriteLine("The process" + Modulelist[2] + " is critical, total shutdown required");
				//PMData->Shutdown.Status = 0xFF;
				//break;
			}

		}
		
		

		// GPS - non critical
		if (PMData->Heartbeat.Flags.GPS == 1) {
			GPSCounter = 0;
			PMData->Heartbeat.Flags.GPS = 0;
			
		}
		else {
			
			// checks if wait time has elapsed
			if (GPSCounter < GPS_WAIT) {
				GPSCounter++;
			}
			else {
				// checks if program is running
				if (ProcessList[3]->HasExited) {
					//if its not running start the program
					Console::WriteLine("The process" + Modulelist[3] + " has exited, starting up");
					ProcessList[3]->Start();
					GPSCounter = 0;
				}
				else {
					// Kill process then start running it again
					Console::WriteLine("The process" + Modulelist[3] + " is being restarted");
					ProcessList[3]->Kill();
					ProcessList[3]->Start();
					Thread::Sleep(100);
					GPSCounter = 0;
				}
			}

		}
		

		
		// Camera - non critical
		if (PMData->Heartbeat.Flags.Camera == 1) {
			CameraCounter = 0;
			PMData->Heartbeat.Flags.Camera = 0;
			Console::WriteLine("Camera is Alive");
		}
		else {
			Console::WriteLine("Camera  wait is " + CameraCounter);
			// checks if wait time has elapsed
			if (CameraCounter < CAMERA_WAIT) {
				CameraCounter++;
			}
			else {
				// checks if program is running
				if (ProcessList[4]->HasExited) {
					//if its not running start the program
					Console::WriteLine("The process" + Modulelist[4] + " has exited, starting up");
					ProcessList[4]->Start();
					CameraCounter = 0;
				}
				else {
					// Kill process then start running it again
					Console::WriteLine("The process" + Modulelist[4] + " is being restarted");
					ProcessList[4]->Kill();
					ProcessList[4]->Start();
					CameraCounter = 0;
				}
			}

		}
		*/

		Thread::Sleep(10);

	}

	Console::ReadKey();
	Console::ReadKey();

	PMData->Shutdown.Status = 0xFF;// put in all other files by replacing Shutdown in the If statement (youll know when u see it otherwise 1:29:00 lecture 2)
	// Initialization
	// Main loop
	//Clearing and Shutdown

	return 0;
}


