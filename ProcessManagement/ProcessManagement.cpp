

#using <System.dll>
#include <conio.h>

//paste all the SMObject into the source files folder for each of the sections
#include <SMObject.h>
#include <smstructs.h>	


using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;

int main() {

	// tele-operation
		// Declaration
	SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));


	array<String^>^ Modulelist = gcnew array<String^>{"Laser", "Display", "Vehicle", "GPS", "Camera"};
	array<int>^ Critical = gcnew array<int>(Modulelist->Length) { 1, 1, 1, 0, 0 };
	array<Process^>^ ProcessList = gcnew array<Process^>(Modulelist->Length);

	//SM Creation and Seeking access
	PMObj.SMCreate();
	PMObj.SMAccess();


	ProcessManagement* PMData = (ProcessManagement*)PMObj.pData;

	for (int i = 0; i < Modulelist->Length; i++) {
		//creates an array of all information processes
		if (Process::GetProcessesByName(Modulelist[i])->Length == 0) {
			ProcessList[i] = gcnew Process;
			ProcessList[i]->StartInfo->FileName = Modulelist[i];
			Console::WriteLine("The process " + Modulelist[i]);
			ProcessList[i]->StartInfo->WorkingDirectory = "../Executable";
			ProcessList[i]->Start();
			Console::WriteLine("The process" + Modulelist[i] + ".exe started");

		}
	}
	// need to deal with priorities

	//Main Loop

	while (!_kbhit()) {
		Thread::Sleep(1000);

	}

	PMData->Shutdown.Status = 0xFF;// put in all other files by replacing Shutdown in the If statement (youll know when u see it otherwise 1:29:00 lecture 2)
	// Initialization
	// Main loop
	//Clearing and Shutdown

	return 0;
}
