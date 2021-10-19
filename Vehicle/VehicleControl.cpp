#include "VehicleControl.h"

int VehicleControl::connect(String^ hostName, int portNumber)
{
	// YOUR CODE HERE
	return 1;
}
int VehicleControl::setupSharedMemory()
{
	ProcessManagementData = new SMObject(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SensorData = new SMObject(_TEXT("VehicleSMObject"), sizeof(SM_VehicleControl));
	ProcessManagementData->SMCreate();
	SensorData->SMCreate();
	ProcessManagementData->SMAccess();
	SensorData->SMAccess();
	PMData = (ProcessManagement*)ProcessManagementData->pData;
	VehicleControl = (SM_VehicleControl*)SensorData->pData;
	PMData->Shutdown.Flags.Laser = 0;

	return 1;
}
int VehicleControl::getData()
{
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