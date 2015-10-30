/*
Author : Mayank Sharma

*/

#include "sceptre.h"
#define HEX 16
#define DEC 10

// Default Device contructor. Sets device's id and default name
Device::Device() {
	id = superId;
	superId++;
	name = "unnamed device";
}
// This constructor sets the given name and id for a device
Device::Device(char* name) {
	id = superId;
	superId++;
	this->name = name;
}
Sceptre::Sceptre(int recv_pin):irrecv(recv_pin) {
	irrecv= IRrecv(recv_pin);

}
int Sceptre::addDevice(Device* device) {
	int returnVal = 1;
	return returnVal;
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
	Code code = activeDevice->gestureCodeMap[myo.getGestureCode()];
	int codeType = code.codeType;
	unsigned long codeValue = code.codeValue;
	unsigned int* rawCodes = code.rawCodes;
	int codeLen = code.codeLen;
	int toggle = code.toggle;
	if (codeType == NEC) {
		if (repeat) {
			irsend.sendNEC(REPEAT, codeLen);
			printDebugMessage("Sent NEC repeat");
		}
		else {
			irsend.sendNEC(codeValue, codeLen);
			printDebugMessage("Sent NEC ");
			printDebugCodeValue(codeValue, HEX);
		}
	}
	else if (codeType == SONY) {
		irsend.sendSony(codeValue, codeLen);
		printDebugMessage("Sent Sony ");
		printDebugCodeValue(codeValue, HEX);
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
			printDebugMessage("Sent RC5 ");
			printDebugCodeValue(codeValue, HEX);
			irsend.sendRC5(codeValue, codeLen);
		}
		else {
			irsend.sendRC6(codeValue, codeLen);
			printDebugMessage("Sent RC6 ");
			printDebugCodeValue(codeValue, HEX);
		}
	}
	else if (codeType == UNKNOWN /* i.e. raw */) {
		// Assume 38 KHz
		irsend.sendRaw(rawCodes, codeLen, 38);
		printDebugMessage("Sent raw");
	}
}
// TODO If null is returned, do nothing
Code* Sceptre::storeCode(decode_results* results) {
	int gestureCode = myo.getGestureCode();
	if (gestureCode == -1) {
		printDebugMessage("Myo was at rest. Please redo gesture and then send the IR signal you want to map to");
		return '\0';
	}
	int codeType = -1; // The type of code
	unsigned long codeValue; // The code value if not raw
	unsigned int rawCodes[RAWBUF]; // The durations if raw
	int codeLen; // The length of the code
	int toggle = 0; // The RC5/6 toggle state

	codeType = results->decode_type;
	int count = results->rawlen;
	if (codeType == UNKNOWN) {
		printDebugMessage("Received unknown code, saving as raw");
		codeLen = results->rawlen - 1;
		// To store raw codes:
		// Drop first value (gap)
		// Convert from ticks to microseconds
		// Tweak marks shorter, and spaces longer to cancel out IR receiver distortion
		for (int i = 1; i <= codeLen; i++) {
			if (i % 2) {
				// Mark
				rawCodes[i - 1] = results->rawbuf[i] * USECPERTICK - MARK_EXCESS;
				printDebugMessage(" m");
			}
			else {
				// Space
				rawCodes[i - 1] = results->rawbuf[i] * USECPERTICK + MARK_EXCESS;
				printDebugMessage(" s");
			}
			printDebugCodeValue(rawCodes[i - 1], DEC);
		}
		printDebugMessage("");
	}
	else {
		if (codeType == NEC) {
			printDebugMessage("Received NEC: ");
			if (results->value == REPEAT) {
				// Don't record a NEC repeat value as that's useless.
				printDebugMessage("repeat; ignoring.");
				return '\0';
			}
		}
		else if (codeType == SONY) {
			printDebugMessage("Received SONY: ");
		}
		else if (codeType == RC5) {
			printDebugMessage("Received RC5: ");
		}
		else if (codeType == RC6) {
			printDebugMessage("Received RC6: ");
		}
		else {
			printDebugMessage("Unexpected codeType ");
			printDebugCodeValue(codeType, DEC);
			printDebugMessage("");
		}
		printDebugCodeValue(results->value, HEX);
		codeValue = results->value;
		codeLen = results->bits;
	}

	Code code = Code();
	code.codeLen = codeLen;
	code.codeType = codeType;
	code.codeValue = codeValue;
	code.toggle = toggle;
	code.rawCodes = rawCodes;
	processing_previous_mapping_request = 1;
	return &code;
	//deviceList[activeDeviceIndex].gestureCodeMap[gestureCode] = code;	
}
// Call in the loop method of arduino. (so it automatically becomes recursive)
// When in training mode, and a code is received processing_previous_mapping_request becomes 1
// then, when the arduino enters training mode again in loop, this method waits for a myo gesture 
// other than rest to map the received code to.
void Sceptre::mapCodeToGesture(Code* code) {
	// if there is no code to be mapped or myo is at rest then return
	int gestureCode = myo.getGestureCode();
	if (!processing_previous_mapping_request || gestureCode == -1)
		return;
	deviceList[activeDeviceIndex].gestureCodeMap[gestureCode] = *code;
}
Myo::Myo() {
	myoController = MyoController();
}
int Myo::getGestureCode() {
	myoController.updatePose();
	int gestureCode = -1;
	switch(myoController.getCurrentPose()) {
	case rest:
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
}



