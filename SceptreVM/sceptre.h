/*
Author : Mayank Sharma

This header file defines the wrappers from MyoController and IRRemote  libraries
*/
#ifndef SPECTRE_H
#define SCEPTRE_H
#include "IRremote.h"
#include "IRremoteInt.h"
#include "MyoController.h"
#define SCEPTRE_NO_OF_DEVICES 1
#define GESTURE_MAP_SIZE 5
//===============================
// Gesture to Array Index Mapping
//===============================
#define DOUBLE_TAP 0
#define FIST 1
#define WAVE_IN 2
#define WAVE_OUT 3
#define FINGER_SPREAD 4
class Myo {
	MyoController myoController;
public:
	Myo();
	Myo(const Myo &myo);
	int getGestureCode();
};
// Storage for the recorded code
class Code {
public:
	Code(const Code &code);
	Code();
	int codeType = -1; // The type of code 
	unsigned long codeValue; // The code value if not raw
	unsigned int* rawCodes; // The durations if raw
	int codeLen= -1; // The length of the code
	int toggle = 0; // The RC5/6 toggle state
	~Code();
};
// Represents a remote controlled device such as TV, AC etc
class Device {
	int id;
public:
	static int superId;
	Device();
	Device(char* name);
	char* name;
	Code* gestureCodeMap;
};
// It combines myo and IR tranciever module and represents the final wearable device capable of 
// sending and receiving ir signals
class Sceptre {
private:
	Device* deviceList;
	int activeDeviceIndex = 0;
	IRsend irsend;
//	int processing_previous_mapping_request = 0;
public:
	Code* tempCode;
	int tempGestureCode;
	Myo myo;
	decode_results results;
	IRrecv irrecv; // decodes the incoming signal
	Sceptre(int recv_pin);
	int addDevice(Device device);
	Device* getActiveDevice();
	int activateDevice(Device* device);
	void sendCode(int repeat);
	Code* storeCode(decode_results *results); // turns on mapping request and returns the code to be mapped
	void saveCurrentMapping();
	Code* decodeAndGetCode();
};
void resetMyoDebugPinsCPP(int mode);
#endif

