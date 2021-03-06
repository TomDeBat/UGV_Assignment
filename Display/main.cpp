
#using <System.dll>
#include <conio.h>

//paste all the SMObject into the source files folder for each of the sections
#include <SMObject.h>
#include <smstructs.h>	


#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <map>

#ifdef __APPLE__
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
	#include <GLUT/glut.h>
	#include <unistd.h>
	#include <sys/time.h>
#elif defined(WIN32)
	#include <Windows.h>
	#include <tchar.h>
	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <GL/glut.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <GL/glut.h>
	#include <unistd.h>
	#include <sys/time.h>
#endif


#include "Camera.hpp"
#include "Ground.hpp"
#include "KeyManager.hpp"

#include "Shape.hpp"
#include "Vehicle.hpp"
#include "MyVehicle.hpp"

#include "Messages.hpp"
#include "HUD.hpp"

#define DEGTORAD (3.141592765 / 180.0)


void display();
void reshape(int width, int height);
void idle();

void keydown(unsigned char key, int x, int y);
void keyup(unsigned char key, int x, int y);
void special_keydown(int keycode, int x, int y);
void special_keyup(int keycode, int x, int y);

void mouse(int button, int state, int x, int y);
void dragged(int x, int y);
void motion(int x, int y);
void drawLaser();
void drawGPS();

using namespace std;
using namespace scos;
using namespace System;
using namespace System::Diagnostics;
using namespace System::Threading;


// Used to store the previous mouse location so we
//   can calculate relative mouse movement.
int prev_mouse_x = -1;
int prev_mouse_y = -1;

// vehicle control related variables
Vehicle * vehicle = NULL;
double speed = 0;
double steering = 0;

SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
SMObject LaserSMObject(TEXT("LaserSMObject"), sizeof(SM_Laser));
SMObject GPSSMObject(TEXT("GPSSMObject"), sizeof(SM_GPS));
SMObject VehicleSMObject(TEXT("VehicleSMObject"), sizeof(SM_VehicleControl));
ProcessManagement* PMData = nullptr;
SM_Laser* LaserData = nullptr;
SM_GPS* GPSData = nullptr;
SM_VehicleControl* VehicleData = nullptr;

//int _tmain(int argc, _TCHAR* argv[]) {
int main(int argc, char ** argv) {

	const int WINDOW_WIDTH = 800;
	const int WINDOW_HEIGHT = 600;
	//Console::WriteLine("set up shared memory");
	// 
	//PMObj.SMCreate();
	//LaserSMObject.SMCreate();
	//GPSSMObject.SMCreate();
	//VehicleSMObject.SMCreate();

	PMObj.SMAccess();
	
	LaserSMObject.SMAccess();

	GPSSMObject.SMAccess();

	VehicleSMObject.SMAccess();

	PMData = (ProcessManagement*)PMObj.pData;
	LaserData = (SM_Laser*)LaserSMObject.pData;
	GPSData = (SM_GPS*)GPSSMObject.pData;
	VehicleData = (SM_VehicleControl*)VehicleSMObject.pData;

	PMData->Shutdown.Flags.Display = 0;

	glutInit(&argc, (char**)(argv));
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("MTRN3500 - GL");

	Camera::get()->setWindowDimensions(WINDOW_WIDTH, WINDOW_HEIGHT);

	glEnable(GL_DEPTH_TEST);

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);

	glutKeyboardFunc(keydown);
	glutKeyboardUpFunc(keyup);
	glutSpecialFunc(special_keydown);
	glutSpecialUpFunc(special_keyup);

	glutMouseFunc(mouse);
	glutMotionFunc(dragged);
	glutPassiveMotionFunc(motion);

	// -------------------------------------------------------------------------
	// Please uncomment the following line of code and replace 'MyVehicle'
	//   with the name of the class you want to show as the current 
	//   custom vehicle.
	// -------------------------------------------------------------------------
	vehicle = new MyVehicle();


	glutMainLoop();

	if (vehicle != NULL) {
		delete vehicle;
	}

	return 0;
}


void display() {

	// -------------------------------------------------------------------------
	//  This method is the main draw routine. 
	// -------------------------------------------------------------------------
	Console::WriteLine("yo doggy");

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if(Camera::get()->isPursuitMode() && vehicle != NULL) {
		double x = vehicle->getX(), y = vehicle->getY(), z = vehicle->getZ();
		double dx = cos(vehicle->getRotation() * 3.141592765 / 180.0);
		double dy = sin(vehicle->getRotation() * 3.141592765 / 180.0);
		Camera::get()->setDestPos(x + (-3 * dx), y + 7, z + (-3 * dy));
		Camera::get()->setDestDir(dx, -1, dy);
	}
	Camera::get()->updateLocation();
	Camera::get()->setLookAt();

	Ground::draw();
	
	// draw my vehicle
	if (vehicle != NULL) {
		vehicle->draw();

	}

	drawLaser();
	drawGPS();

	// draw HUD
	HUD::Draw();

	glutSwapBuffers();
};

void drawLaser() {
	static GLUquadric* laserQuad = gluNewQuadric();
	double x = 0, y = vehicle->getY() + 0.03, z = 0;
	glPushMatrix();
	for (int i = 1; i < STANDARD_LASER_LENGTH; i++) {
		glColor3f(0.3, 0.7, 0.8);
		glTranslatef(LaserData->x[i]/1000 - x, y, LaserData->y[i]/1000 - z);
		x = LaserData->x[i] / 1000;
		z = LaserData->y[i] / 1000;
		y = 0;
		Console::WriteLine(" {0, 4:F3}  {1, 4:F3}  {2, 4:F3} ", LaserData->x[i] / 1000 + x, 0.3, LaserData->y[i] / 1000 + y);
		glPushMatrix();
		glRotatef(-90, 1, 0, 0);
		gluCylinder(laserQuad, 0.02, 0.02, 1, 12, 1);
		glPopMatrix();
	}

	glPopMatrix();
	glEnd();

}

void drawGPS() {

	Camera::get()->switchTo2DDrawing();
	int winWidthOff = (Camera::get()->getWindowWidth() - 800) * .5;
	if (winWidthOff < 0)
		winWidthOff = 0;

	glColor3f(0.3, 0.7, 0.4);
	double x = 130 + winWidthOff;
	double y = 600;
	char GPSLabel[250];
	sprintf(GPSLabel, "Northing: %.4f  Easting: %.4f  Height: %.4f", GPSData->northing,
		GPSData->easting, GPSData->height);

	double r = 4;

	glPushMatrix();
	double r1 = r;
	double r2 = r * 1.05;

	const double centerR = -90;
	const double startR = centerR - 50;
	const double endR = centerR + 50;

	glTranslatef(x, y, 0);
	glDisable(GL_LIGHTING);

	y = sin((startR)*DEGTORAD);
	// text label
	//renderString(label, strlen(label) * 10 * -.25, -r1 + 20, GLUT_BITMAP_HELVETICA_10);
	HUD::RenderString(GPSLabel, strlen(GPSLabel) * 10 * -.25, (r1 - 20) * y - 20, GLUT_BITMAP_HELVETICA_18);

	glPopMatrix();

	Camera::get()->switchTo3DDrawing();

}


void reshape(int width, int height) {

	Camera::get()->setWindowDimensions(width, height);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
};

double getTime()
{
#if defined(WIN32)
	LARGE_INTEGER freqli;
	LARGE_INTEGER li;
	if(QueryPerformanceCounter(&li) && QueryPerformanceFrequency(&freqli)) {
		return double(li.QuadPart) / double(freqli.QuadPart);
	}
	else {
		static ULONGLONG start = GetTickCount64();
		return (GetTickCount64() - start) / 1000.0;
	}
#else
	struct timeval t;
	gettimeofday(&t, NULL);
	return t.tv_sec + (t.tv_usec / 1000000.0);
#endif
}

// put code in here
void idle() {

	if (PMData->Shutdown.Flags.Display) exit(0);
	//Console::WriteLine("I have reached here");
	PMData->Heartbeat.Flags.Display = 1;
	//PMData->PMHeartbeat.Flags.Display = 1; // for debugging

	if (PMData->PMHeartbeat.Flags.Display == 1) {
		PMData->PMHeartbeat.Flags.Display = 0;
		PMData->PMCounter[DISPLAY_POS] = 0;
		//Console::WriteLine("ye");
	}
	else {
		if (PMData->PMCounter[DISPLAY_POS] > PM_WAIT) {
			PMData->Shutdown.Status = 0xFF;
			//Console::WriteLine("Complete");
		}
		else {
			PMData->PMCounter[DISPLAY_POS]++;
		}
	}

	if (KeyManager::get()->isAsciiKeyPressed('a')) {
		Camera::get()->strafeLeft();
	}

	if (KeyManager::get()->isAsciiKeyPressed('c')) {
		Camera::get()->strafeDown();
	}

	if (KeyManager::get()->isAsciiKeyPressed('d')) {
		Camera::get()->strafeRight();
	}

	if (KeyManager::get()->isAsciiKeyPressed('s')) {
		Camera::get()->moveBackward();
	}

	if (KeyManager::get()->isAsciiKeyPressed('w')) {
		Camera::get()->moveForward();
	}

	if (KeyManager::get()->isAsciiKeyPressed(' ')) {
		Camera::get()->strafeUp();
	}

	speed = 0;
	steering = 0;

	if (KeyManager::get()->isSpecialKeyPressed(GLUT_KEY_LEFT)) {
		steering = Vehicle::MAX_LEFT_STEERING_DEGS * -1;
	}

	if (KeyManager::get()->isSpecialKeyPressed(GLUT_KEY_RIGHT)) {
		steering = Vehicle::MAX_RIGHT_STEERING_DEGS * -1;
	}

	if (KeyManager::get()->isSpecialKeyPressed(GLUT_KEY_UP)) {
		speed = Vehicle::MAX_FORWARD_SPEED_MPS;

	}

	if (KeyManager::get()->isSpecialKeyPressed(GLUT_KEY_DOWN)) {
		speed = Vehicle::MAX_BACKWARD_SPEED_MPS;
	}

	VehicleData->Steering = steering;
	VehicleData->Speed = speed;

	const float sleep_time_between_frames_in_seconds = 0.025;

	static double previousTime = getTime();
	const double currTime = getTime();
	const double elapsedTime = currTime - previousTime;
	previousTime = currTime;

	// do a simulation step
	if (vehicle != NULL) {
		vehicle->update(speed, steering, elapsedTime);
	}

	display();

#ifdef _WIN32 
	Sleep(sleep_time_between_frames_in_seconds * 1000);
#else
	usleep(sleep_time_between_frames_in_seconds * 1e6);
#endif
};

void keydown(unsigned char key, int x, int y) {

	// keys that will be held down for extended periods of time will be handled
	//   in the idle function
	KeyManager::get()->asciiKeyPressed(key);

	// keys that react ocne when pressed rather than need to be held down
	//   can be handles normally, like this...
	switch (key) {
	case 27: // ESC key
		exit(0);
		break;      
	case '0':
		Camera::get()->jumpToOrigin();
		break;
	case 'p':
		Camera::get()->togglePursuitMode();
		break;
	}

};

void keyup(unsigned char key, int x, int y) {
	KeyManager::get()->asciiKeyReleased(key);
};

void special_keydown(int keycode, int x, int y) {

	KeyManager::get()->specialKeyPressed(keycode);

};

void special_keyup(int keycode, int x, int y) {  
	KeyManager::get()->specialKeyReleased(keycode);  
};

void mouse(int button, int state, int x, int y) {

};

void dragged(int x, int y) {

	if (prev_mouse_x >= 0) {

		int dx = x - prev_mouse_x;
		int dy = y - prev_mouse_y;

		Camera::get()->mouseRotateCamera(dx, dy);
	}

	prev_mouse_x = x;
	prev_mouse_y = y;
};

void motion(int x, int y) {

	prev_mouse_x = x;
	prev_mouse_y = y;
};


