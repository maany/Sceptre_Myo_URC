/*
Author : Mayank Sharma

*/

#include "sceptre.h"
#define HEX 16
#define DEC 10

int SEND_MODE_DETECTED_MYO_GESTURE = 9;
Code::Code(const Code& code) {
	Serial.println("Copy constructor invoked maahn");
	codeType = code.codeType;
	codeLen = code.codeLen;
	codeValue = code.codeValue;
	rawCodes = code.rawCodes;
	toggle = code.toggle;
}
Code::Code() {
	codeType = -1;
	codeLen = -1;
	toggle = 0;
	rawCodes = new unsigned int;
	codeValue = -1;
}
int Device::superId = 0;
// Default Device contructor. Sets device's id and default name
Device::Device() {
	id = superId;
	superId = superId + 1;
	name = "unnamed device";
	gestureCodeMap = new Code[GESTURE_MAP_SIZE];
}
// This constructor sets the given name and id for a device
Device::Device(char* name) {
	id = superId;
	superId++;
	this->name = name;
	gestureCodeMap = new Code[GESTURE_MAP_SIZE];
}
Sceptre::Sceptre(int recv_pin):irrecv(recv_pin) {
	irrecv= IRrecv(recv_pin);
	deviceList = new Device[SCEPTRE_NO_OF_DEVICES];
}
// TODO refine logic, detect overflow
int Sceptre::addDevice(Device device) {
	if (activeDeviceIndex <= 0)
		activeDeviceIndex = 0;
	else if (activeDeviceIndex < SCEPTRE_NO_OF_DEVICES - 1)
		activeDeviceIndex++;
	else {
		return 0;
	}
	deviceList[activeDeviceIndex] = device;
	return 1;
}
int Sceptre::activateDevice(Device* device) {
	int returnVal = 1;
	return returnVal;
}
Device* Sceptre::getActiveDevice() {
	return &deviceList[activeDeviceIndex];
}
void Sceptre::sendCode(int repeat) {
	Device* activeDevice = &deviceList[activeDeviceIndex];
	
	int gestureCode;
	gestureCode = WAVE_OUT;
	/*
	gestureCode = myo.getGestureCode();
	switch (gestureCode) {
	case -1: 
		Serial.println("myo at rest");
		digitalWrite(SEND_MODE_DETECTED_MYO_GESTURE, LOW);
		::resetMyoDebugPinsCPP(LOW);
		return;
	case DOUBLE_TAP: ::resetMyoDebugPinsCPP(LOW); digitalWrite(DOUBLE_TAP + 2, HIGH); Serial.println("Double Tap"); break;
	case FIST: ::resetMyoDebugPinsCPP(LOW); digitalWrite(12, HIGH); Serial.println("Fist"); break;
	case WAVE_IN: ::resetMyoDebugPinsCPP(LOW); digitalWrite(WAVE_IN + 2, HIGH); Serial.println("Wave In");  break;
	case WAVE_OUT: ::resetMyoDebugPinsCPP(LOW); digitalWrite(WAVE_OUT + 2, HIGH); Serial.println("Wave_Out");  break;
	case FINGER_SPREAD: ::resetMyoDebugPinsCPP(LOW); digitalWrite(FINGER_SPREAD + 2, HIGH); Serial.println("Finger Spread"); break;
	}
	*/
	delay(2000);
	Code code = activeDevice->gestureCodeMap[gestureCode];
	::resetMyoDebugPinsCPP(HIGH);
	delay(100);
	::resetMyoDebugPinsCPP(LOW);
	delay(100);
	::resetMyoDebugPinsCPP(HIGH);
	delay(100);
	::resetMyoDebugPinsCPP(LOW);
	delay(100);
	::resetMyoDebugPinsCPP(HIGH);
	int codeType = code.codeType;
	unsigned long codeValue = code.codeValue;
	unsigned int* rawCodes = code.rawCodes;
	int codeLen = code.codeLen;
	int toggle = code.toggle;
	if (codeType == NEC) {
		if (repeat) {
			irsend.sendNEC(REPEAT, codeLen);
			Serial.println("Sent NEC repeat");
		}
		else {
			irsend.sendNEC(codeValue, codeLen);
			Serial.print("Sent NEC ");
			Serial.println(codeValue, HEX);
		}
	}
	else if (codeType == SONY) {
		irsend.sendSony(codeValue, codeLen);
		Serial.print("Sent Sony ");
		Serial.println(codeValue, HEX);
	}
	else if (codeType == RC5 || codeType == RC6) {
		if (!repeat) {
			// Flip the toggle bit for a new button press
			toggle = 1 - toggle;
		}
		// Put the toggle bit into the code to send
		codeValue = codeValue & ~(1 << (codeLen - 1));
		codeValue = codeValue | (toggle << (codeLen - 1));
		if (codeType == RC5) {
			Serial.print("Sent RC5 ");
			Serial.println(codeValue, HEX);
			irsend.sendRC5(codeValue, codeLen);
		}
		else {
			irsend.sendRC6(codeValue, codeLen);
			Serial.print("Sent RC6 ");
			Serial.println(codeValue, HEX);
		}
	}
	else if (codeType == UNKNOWN /* i.e. raw */) {
		// Assume 38 KHz
		irsend.sendRaw(rawCodes, codeLen, 38);
		Serial.println("Sent raw");
	}
	::resetMyoDebugPinsCPP(LOW);
}
// Stores the code for later playback
// Most of this code is just logging
Code* Sceptre::storeCode(decode_results* results) {

	int codeType = -1; // The type of code
	unsigned long codeValue; // The code value if not raw
	unsigned int rawCodes[RAWBUF]; // The durations if raw
	int codeLen; // The length of the code
	int toggle = 0; // The RC5/6 toggle state
	codeType = results->decode_type;
	int count = results->rawlen;
	if (codeType == UNKNOWN) {
		Serial.println("Received unknown code, saving as raw");
		codeLen = results->rawlen - 1;
		// To store raw codes:
		// Drop first value (gap)
		// Convert from ticks to microseconds
		// Tweak marks shorter, and spaces longer to cancel out IR receiver distortion
		for (int i = 1; i <= codeLen; i++) {
			if (i % 2) {
				// Mark
				rawCodes[i - 1] = results->rawbuf[i] * USECPERTICK - MARK_EXCESS;
				Serial.print(" m");
			}
			else {
				// Space
				rawCodes[i - 1] = results->rawbuf[i] * USECPERTICK + MARK_EXCESS;
				Serial.print(" s");
			}
			Serial.print(rawCodes[i - 1], DEC);
		}
		Serial.println("");
	}
	else {
		if (codeType == NEC) {
			Serial.print("Received NEC: ");
			if (results->value == REPEAT) {
				// Don't record a NEC repeat value as that's useless.
				Serial.println("repeat; ignoring.");
				return '\0';
			}
		}
		else if (codeType == SONY) {
			Serial.print("Received SONY: ");
		}
		else if (codeType == RC5) {
			Serial.print("Received RC5: ");
		}
		else if (codeType == RC6) {
			Serial.print("Received RC6: ");
		}
		else {
			Serial.print("Unexpected codeType ");
			Serial.print(codeType, DEC);
			Serial.println("");
		}
		Serial.println(results->value, HEX);
		codeValue = results->value;
		codeLen = results->bits;
	}

	Code *code = new Code();
	code->codeLen = codeLen;
	code->codeType = codeType;
	code->codeValue = results->value;
	Serial.print("code value from inside : "); Serial.println(code->codeValue,HEX);
	Serial.print("result value from inside : "); Serial.println(results->value, HEX);
	code->toggle = toggle;
	code->rawCodes = rawCodes;
	//processing_previous_mapping_request = 1;
	return code;
	//deviceList[activeDeviceIndex].gestureCodeMap[gestureCode] = code;	
}

void Sceptre::saveCurrentMapping() {
	getActiveDevice()->gestureCodeMap[tempGestureCode] = *tempCode;
	Serial.print("Mapping stored is : "); Serial.print(tempGestureCode); Serial.print(" "); Serial.println(getActiveDevice()->gestureCodeMap[tempGestureCode].codeValue);
}
// TODO To check if code is invalid, codeLength will be 0 or lesser
Code* Sceptre::decodeAndGetCode() {
	irrecv.decode(&results);
	Code* code = storeCode(&results);
	return code;
}
Code::~Code() {
	Serial.println("Object is being destroyed");
}
Myo::Myo() {
	myoController = MyoController();
}
Myo::Myo(const Myo& myo) {
	myoController = myo.myoController;
}
int Myo::getGestureCode() {
	myoController.updatePose();
	int gestureCode = -1;
	switch(myoController.getCurrentPose()) {
	case rest: gestureCode = -1;
		break;
	case fist:
		gestureCode = FIST;
		break;
	case doubleTap:
		gestureCode = DOUBLE_TAP;
		break;
	case waveIn:
		gestureCode = WAVE_IN;
		break;
	case waveOut:
		gestureCode = WAVE_OUT;
		break;
	case fingersSpread:
		gestureCode = FINGER_SPREAD;
		break;
	}
	return gestureCode;
}
void resetMyoDebugPinsCPP(int mode) {
	digitalWrite(DOUBLE_TAP + 2, mode);
	digitalWrite(12, mode);
	digitalWrite(WAVE_IN + 2, mode);
	digitalWrite(WAVE_OUT + 2, mode);
	digitalWrite(FINGER_SPREAD + 2, mode);
}



