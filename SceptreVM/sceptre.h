/*
Author : Mayank Sharma

This header file defines the wrappers from MyoController and IRRemote  libraries
*/
#ifndef SPECTRE
#define SCEPTRE
#include<IRremote.h>
#include<MyoController.h>
#define SCEPTRE_NO_OF_DEVICES 5
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
	int getGestureCode();
};
// Storage for the recorded code
class Code {
public:
	int codeType = -1; // The type of code
	unsigned long codeValue; // The code value if not raw
	unsigned int* rawCodes; // The durations if raw
	int codeLen; // The length of the code
	int toggle = 0; // The RC5/6 toggle state
};
// Represents a remote controlled device such as TV, AC etc
class Device {
	int id;
public:
	static int superId;
	Device();
	Device(char* name);
	char* name;
	Code gestureCodeMap[GESTURE_MAP_SIZE];
};
// It combines myo and IR tranciever module and represents the final wearable device capable of 
// sending and receiving ir signals
class Sceptre {
private:
	Device deviceList[SCEPTRE_NO_OF_DEVICES];
	int activeDeviceIndex = 0;
	IRsend irsend;
	decode_results results;
	Myo myo;
	int processing_previous_mapping_request = 0;
public:
	IRrecv irrecv; // decodes the incoming signal
	Sceptre(int recv_pin);
	int addDevice(Device* device);
	Device* getActiveDevice();
	int activateDevice(Device* device);
	void sendCode(int repeat);
	Code* storeCode(decode_results *results); // turns on mapping request and returns the code to be mapped
	void mapCodeToGesture(Code* code);// wait till a gesture is received, then map the code to it as per mapping request
	Code decodeAndGetCode();
};
#endif

