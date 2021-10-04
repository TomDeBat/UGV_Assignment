#pragma once

#ifndef SMSTRUCTS_H
#define SMSTRUCTS_H

#using <System.dll>
#include <Windows.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <iostream>
#include <conio.h>

using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;


#define STANDARD_LASER_LENGTH 361
#define LASER_WAIT 60
#define GPS_WAIT 60
#define VEHICLE_WAIT 60
#define CAMERA_WAIT 60
#define DISPLAY_WAIT 60
#define PM_WAIT 100
#define PI_DEF 3.14159

struct SM_Laser
{
	double x[STANDARD_LASER_LENGTH];
	double y[STANDARD_LASER_LENGTH];
};

struct SM_VehicleControl
{
	double Speed;
	double Steering;
};

struct SM_GPS
{
	double northing;
	double easting;
	double height;
};

struct UnitFlags
{
	unsigned char	ProcessManagement : 1,	//NONCRITICAL
					Laser : 1,				//NONCRITICAL
					Display : 1,			//NONCRITICAL
					VehicleControl : 1,		//NONCRITICAL
					GPS : 1,				//NONCRITICAL
					Camera : 1,				//NONCRITICAL
					Garbage : 3;
};

union ExecFlags
{
	UnitFlags Flags;
	unsigned short Status;
};

struct ProcessManagement
{
	ExecFlags Heartbeat;
	ExecFlags Shutdown;
	ExecFlags PMHeartbeat;
	int PMCounter[5];
	long int LifeCounter;
};

#define NONCRITICALMASK 0xff	//0 011 0000
#define CRITICALMASK 0x0		//0 100 1111
#endif
