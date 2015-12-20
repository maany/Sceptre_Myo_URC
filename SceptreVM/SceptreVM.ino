/*
 Name:		SceptreVM.ino
 Created:	10/30/2015 8:51:45 AM
 Author:	Mayank
*/

// the setup function runs once when you press reset or power the board
#include <MyoController.h>
#include <IRremoteInt.h>
#include <IRremote.h>
#include <sceptre.h>

int RECV_PIN = 11;
int BUTTON_PIN = 12;
int STATUS_PIN = 13;
int MODE_SELECT_PIN = 5;
int RECV_MODE_PIN = 6;
int RECV_MODE_WAITING_FOR_MYO_PIN = 7;
int SEND_MODE_PIN = 8;

Sceptre sceptre(RECV_PIN);
decode_results results;
void setup() {
	Serial.begin(9600);
	sceptre.irrecv.enableIRIn(); // Start the receiver
	pinMode(BUTTON_PIN, INPUT);
	pinMode(STATUS_PIN, OUTPUT);
	pinMode(SEND_MODE_PIN, OUTPUT);
	pinMode(RECV_MODE_PIN, OUTPUT);
	pinMode(RECV_MODE_PIN, OUTPUT);
	pinMode(MODE_SELECT_PIN,INPUT);
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
		digitalWrite(RECV_MODE_PIN, HIGH);
		sceptre.irrecv.enableIRIn();
		Code code = sceptre.decodeAndGetCode();
		digitalWrite(RECV_MODE_PIN, LOW);
	}
	//if myo was not at rest and then it comes to rest, 
	int buttonState = digitalRead(BUTTON_PIN);
	if (lastButtonState == HIGH && buttonState == LOW) {
		Serial.println("Released");
		//sceptre.irrecv.enableIRIn(); // Re-enable receiver
	}
	// if myo was at rest and then some gesture was detected, send the mapped code
	if (buttonState) {
		Serial.println("Pressed, sending");
		digitalWrite(STATUS_PIN, HIGH);
		sceptre.sendCode(lastButtonState == buttonState);
		digitalWrite(STATUS_PIN, LOW);
		delay(50); // Wait a bit between retransmissions
	}
	// myo at rest
	else if (sceptre.irrecv.decode(&results)) {
		digitalWrite(STATUS_PIN, HIGH);
		sceptre.storeCode(&results);
		sceptre.irrecv.resume(); // resume receiver
		digitalWrite(STATUS_PIN, LOW);
	}
	lastButtonState = buttonState;
}
