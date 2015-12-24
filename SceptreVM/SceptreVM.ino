/*
 Name:		SceptreVM.ino
 Created:	10/30/2015 8:51:45 AM
 Author:	Mayank
*/

// the setup function runs once when you press reset or power the board
#include <IRremoteInt.h>
#include <IRremote.h>
#include <MyoController.h>
#include <sceptre.h>

int RECV_PIN = 11;
int FIST_LED = 12;
int STATUS_PIN = 13;
int MODE_SELECT_PIN = 7;
int RECV_MODE_PIN = 8;
int RECV_MODE_WAITING_FOR_MYO_PIN = 9;
int PROCEED_SIGNAL_PIN = 10;
// Device name
#define DEMO_DEVICE_NAME "device1"

int count = 0;
Sceptre sceptre(RECV_PIN);
decode_results results;
void setup() {
	Serial.begin(9600);
	sceptre.irrecv.enableIRIn(); // Start the receiver
	//pinMode(BUTTON_PIN, INPUT); //Todo might not need button, as button was used to send code
	pinMode(STATUS_PIN, OUTPUT);
	pinMode(PROCEED_SIGNAL_PIN,INPUT);
	pinMode(RECV_MODE_PIN, OUTPUT);
	pinMode(MODE_SELECT_PIN,INPUT);
	sceptre.results.value = -1;
	//add a new Device, since this is the first device added, it is activated by default
	Device device = Device(DEMO_DEVICE_NAME);
	sceptre.addDevice(device);
}
int lastButtonState;
// the loop function runs over and over again until power down or reset
void loop() {
	// If button pressed, send the code.
	// button pressed means that myo is not at rest
	int mode = digitalRead(MODE_SELECT_PIN);
	//send mode
	if (mode == 1) {
		digitalWrite(DOUBLE_TAP +2, HIGH);
		digitalWrite(FIST_LED, HIGH);
		int gestureCode;
		gestureCode = sceptre.myo.getGestureCode();
		//gestureCode = WAVE_OUT;
		Serial.println(sceptre.getActiveDevice()->gestureCodeMap[gestureCode].codeValue,HEX);
		
		sceptre.sendCode(1); // takes care of identifying myo gesture
		
		digitalWrite(DOUBLE_TAP + 2, LOW);
		digitalWrite(FIST_LED, LOW);
		
		delay(1000);
		//digitalWrite(2, LOW);

	}
	else {
		//receive mode
		// Step 1 : read code from IR receiver 
		// 2 stage code locking. i.e send same code twice to proceed. This is essential to avoid noise
		digitalWrite(RECV_MODE_PIN, HIGH);
		
		sceptre.irrecv.enableIRIn();
		Code* code;
		unsigned long prevCodeValue=1;
		do
		{
			if (sceptre.irrecv.decode(&results)) {
				prevCodeValue = code->codeValue;
				code = sceptre.storeCode(&results);
				Serial.print("Previous Value : "); Serial.println(prevCodeValue, HEX);
				Serial.print("Oustide file : "); Serial.println(code->codeValue, HEX);
				sceptre.irrecv.resume();
				delay(100);
			}
		} while (prevCodeValue!=code->codeValue);
		Serial.print("Code received for mapping is : "); Serial.println(code->codeValue,HEX);
		sceptre.tempCode = code;
		digitalWrite(RECV_MODE_PIN, LOW);
		
		// Press button to begin gesture mapping i.e enter step 2
		//Step 2: detect Myo gesture
		digitalWrite(RECV_MODE_WAITING_FOR_MYO_PIN, HIGH);
		int gestureCode=-1;
		/*
		switch (count) {
		case 0: gestureCode = FIST; break;
		case 1: gestureCode = WAVE_OUT; break;
		case 2: gestureCode = WAVE_IN; break;
		}
		count++;
		*/
		
		Myo* myo = &sceptre.myo;
		do{
			gestureCode = myo->getGestureCode();
			delay(100);
		} while (gestureCode == -1);
		sceptre.tempGestureCode = gestureCode;
		
		// Debug via LED's
		switch (gestureCode) {
		case DOUBLE_TAP: resetMyoDebugPins(LOW); digitalWrite(DOUBLE_TAP + 2, HIGH); Serial.println("Double Tap"); break;
		case FIST: resetMyoDebugPins(LOW); digitalWrite(FIST_LED, HIGH);Serial.println("Fist"); break;
		case WAVE_IN: resetMyoDebugPins(LOW); digitalWrite(WAVE_IN +2, HIGH); Serial.println("Wave In");  break;
		case WAVE_OUT: resetMyoDebugPins(LOW); digitalWrite(WAVE_OUT + 2, HIGH); Serial.println("Wave_Out");  break;
		case FINGER_SPREAD: resetMyoDebugPins(LOW); digitalWrite(FINGER_SPREAD + 2, HIGH); Serial.println("Finger Spread"); break;
		}
		
		//delay(5000);
		//gestureCode = WAVE_OUT;
		digitalWrite(RECV_MODE_WAITING_FOR_MYO_PIN, LOW);
		//Step 3 Store gesture
		
		//sceptre.saveCurrentMapping();
		sceptre.getActiveDevice()->gestureCodeMap[gestureCode] = *code;
		Serial.print("Mapping stored is : "); Serial.print(gestureCode); Serial.print(" "); Serial.println(sceptre.getActiveDevice()->gestureCodeMap[gestureCode].codeValue,HEX);
		// Set MODE_SELECT_PIN , then make PROCEED_SIGNAL_PIN high to proceed 
		// if training is complete and you want to enter Send Mode, make MODE_SELECT_PIN high and give the PROCEED_SIGNAL
		// if you want to continue in Training mode, make sure MODE_SELECT_PIN is low and give the PROCEED_SIGNAL
		digitalWrite(RECV_MODE_WAITING_FOR_MYO_PIN, HIGH);
		digitalWrite(RECV_MODE_PIN, HIGH);
		resetMyoDebugPins(HIGH);
		while (digitalRead(PROCEED_SIGNAL_PIN) == HIGH) {
			//Serial.println("Waiting");
			delay(100);
		}
		digitalWrite(RECV_MODE_WAITING_FOR_MYO_PIN, LOW);
		digitalWrite(RECV_MODE_PIN, LOW);
		resetMyoDebugPins(LOW);
		//if myo was not at rest and then it comes to rest, 
		

	}

}
void resetMyoDebugPins(int mode) {
	digitalWrite(DOUBLE_TAP + 2, mode);
	digitalWrite(FIST_LED, mode);
	digitalWrite(WAVE_IN + 2, mode);
	digitalWrite(WAVE_OUT + 2, mode);
	digitalWrite(FINGER_SPREAD + 2,mode);
}