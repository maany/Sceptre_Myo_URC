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
int BUTTON_PIN = 12;
int STATUS_PIN = 13;
int MODE_SELECT_PIN = 7;
int RECV_MODE_PIN = 8;
int RECV_MODE_WAITING_FOR_MYO_PIN = 9;
int SEND_MODE_PIN = 10;
// Debug pins for myo gesture


Sceptre sceptre(RECV_PIN);
decode_results results;
void setup() {
	Serial.begin(9600);
	sceptre.irrecv.enableIRIn(); // Start the receiver
	pinMode(BUTTON_PIN, INPUT); //Todo might not need button, as button was used to send code
	pinMode(STATUS_PIN, OUTPUT);
	pinMode(SEND_MODE_PIN, OUTPUT);
	pinMode(RECV_MODE_PIN, OUTPUT);
	pinMode(MODE_SELECT_PIN,INPUT);
	sceptre.results.value = -1;
}
int lastButtonState;
// the loop function runs over and over again until power down or reset
void loop() {
	// If button pressed, send the code.
	// button pressed means that myo is not at rest
	int mode = digitalRead(MODE_SELECT_PIN);
	//send mode
	if (mode == 1) {
		digitalWrite(SEND_MODE_PIN, HIGH);
		sceptre.sendCode(1); // takes care of identifying myo gesture
		digitalWrite(SEND_MODE_PIN, LOW);

	}
	else {
		//receive mode
		// Step 1 : read code from IR receiver 
		digitalWrite(RECV_MODE_PIN, HIGH);
		/*
		sceptre.irrecv.enableIRIn();
		Code* code;
		do
		{
			if (sceptre.irrecv.decode(&results)) {
				code = sceptre.storeCode(&results);
				//Serial.print("Oustide file : "); Serial.println(code->codeValue, HEX);
				sceptre.irrecv.resume();
				delay(100);
			}
		} while (code->codeType == -1);
		Serial.print("Code received is : "); Serial.println(code->codeValue,HEX);
		*/
		digitalWrite(RECV_MODE_PIN, LOW);
		
		// Press button to begin gesture mapping i.e enter step 2
		//Step 2: detect Myo gesture
		digitalWrite(RECV_MODE_WAITING_FOR_MYO_PIN, HIGH);
		Myo* myo = &sceptre.myo;
		int gestureCode;
		do{
			gestureCode = myo->getGestureCode();
			delay(100);
		} while (gestureCode == -1);

		// Debug via LED's
		switch (gestureCode) {
		case DOUBLE_TAP: resetMyoDebugPins(); digitalWrite(DOUBLE_TAP + 2, HIGH); Serial.println("Double Tap"); break;
		case FIST: resetMyoDebugPins(); digitalWrite(FIST +2, HIGH);Serial.println("Fist"); break;
		case WAVE_IN: resetMyoDebugPins(); digitalWrite(WAVE_IN +2, HIGH); Serial.println("Wave In");  break;
		case WAVE_OUT: resetMyoDebugPins(); digitalWrite(WAVE_OUT + 2, HIGH); Serial.println("Wave_Out");  break;
		case FINGER_SPREAD: resetMyoDebugPins(); digitalWrite(FINGER_SPREAD + 2, HIGH); Serial.println("Finger Spread"); break;
		}
		while (1) {

		}
		digitalWrite(RECV_MODE_WAITING_FOR_MYO_PIN, LOW);
		//if myo was not at rest and then it comes to rest, 
		

	}

}
void resetMyoDebugPins() {
	digitalWrite(DOUBLE_TAP + 2, LOW);
	digitalWrite(FIST + 2, LOW);
	digitalWrite(WAVE_IN + 2, LOW);
	digitalWrite(WAVE_OUT + 2, LOW);
	digitalWrite(FINGER_SPREAD + 2,LOW);
}