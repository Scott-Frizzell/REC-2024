#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <ArduinoQueue.h>
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_I2CRegister.h>
#include <Adafruit_SPIDevice.h>
#include "NRFMessage.h"
#include <Servo.h>
#include "TrackSection.h"

// CONSTANTS
#define BAUD_RATE 9600
#define BUTTON_PRESSED HIGH
#define BUTTON_UNPRESSED LOW
#define SWITCH_ON HIGH
#define SWITCH_OFF LOW
#define LED_ENABLED LOW
#define LED_DISABLED HIGH

// ARDUINO PINS
#define PIN_CE 7
#define PIN_CSN 8
#define PIN_BRAKE0_SERVO 10
#define PIN_BRAKE1_SERVO 4
#define PIN_BRAKE2_SERVO 5
#define PIN_BRAKE3_SERVO 6
#define PIN_BRAKE1_HALL A2
#define PIN_BRAKE1_HALL A1
#define PIN_BRAKE1_HALL A0
#define PIN_ESTOP_INT 2
#define PIN_ESTOP_REMOTE_INT 3
#define PIN_THEMING_SERVO A3

// EXPANDER PINS
#define PIN_ESTOP_BUTTON 0
#define PIN_BRAKE3_SWITCH 1
#define PIN_START_BUTTON 8
#define PIN_BRAKE1_LED 9
#define PIN_RESET_BUTTON 10
#define PIN_BRAKE1_SWITCH 11
#define PIN_BRAKE2_LED 12
#define PIN_THEMING_BUTTON 13
#define PIN_BRAKE2_SWITCH 14
#define PIN_BRAKE3_LED 15

// PARAMETERS
#define SBRAKE0_ENGAGED 0
#define SBRAKE0_DISENGAGED 90
#define SBRAKE1_ENGAGED 0
#define SBRAKE1_DISENGAGED 90
#define SBRAKE2_ENGAGED 0
#define SBRAKE2_DISENGAGED 90
#define SBRAKE3_ENGAGED 0
#define SBRAKE3_DISENGAGED 90
#define THEMING_SPEED 50
#define SECTION_NUMBER 4
#define CHECKPOINT_NUMBER 10
#define SECTION0_BRAKE 1
#define SECTION0_END 3
#define SECTION1_BRAKE 5
#define SECTION1_END 6
#define SECTION2_BRAKE 8
#define SECTION2_END 9
#define SECTION3_BRAKE -1
#define SECTION3_END 10
#define LONG_PRESS_DURATION 2000

int states[10];
ArduinoQueue<NRFMessage> outgoingQueue = ArduinoQueue<NRFMessage>(10);
ArduinoQueue<NRFMessage> incomingQueue = ArduinoQueue<NRFMessage>(10);
RF24 radio(PIN_CE, PIN_CSN);
const byte write_addr[6] = "00001";
const byte read_addr[6] = "00002";
const Servo SBrake0;
const Servo SBrake1;
const Servo SBrake2;
const Servo SBrake3;
const Servo Theming;
const Adafruit_MCP23X17 expander;
const TrackSection track[SECTION_NUMBER];
const Checkpoint checkpoints[CHECKPOINT_NUMBER];
bool lastStateStart;
bool lastStateReset;
bool lastStateTheming;
bool lastStateSwitch0;
bool lastStateSwitch1;
bool lastStateSwitch2;
int startHoldDown;


void setup() {
	radio.begin();
}

void loop() {
	// TASK 0 | GENERAL RIDE FLOW
	switch (states[0]) {
		case -1: // STATE E-STOP
			break;
		case 0: // STATE 0 | INIT
			expander.beginI2C();
			expander.pinMode(PIN_ESTOP_BUTTON, INPUT);
			expander.setupInterruptPin(PIN_ESTOP_BUTTON, HIGH);
			expander.pinMode(PIN_BRAKE3_SWITCH, INPUT);
			expander.pinMode(PIN_START_BUTTON, INPUT);
			expander.pinMode(PIN_BRAKE1_LED, OUTPUT);
			expander.pinMode(PIN_RESET_BUTTON, INPUT);
			expander.pinMode(PIN_BRAKE1_SWITCH, INPUT);
			expander.pinMode(PIN_BRAKE2_LED, OUTPUT);
			expander.pinMode(PIN_THEMING_BUTTON, INPUT);
			expander.pinMode(PIN_BRAKE2_SWITCH, INPUT);
			expander.pinMode(PIN_BRAKE3_LED, OUTPUT);
			
			break;
		case 1: // STATE 1 | STOPPED
			break;
		case 2: // STATE 2 | RUNNING
			break;
	}
	
	// TASK 1 | NRF SEND
	switch (states[1]) {
		case -1: // STATE E-STOP
			char msg[] = "ESTOP"; // TODO
			Serial.print("Sending: ");
			Serial.println(msg);
			radio.stopListening();
			radio.write(&msg[0], sizeof(msg));
			radio.startListening();
			states[1] = 1;
			break;
		case 0: // STATE 0 | INIT
			radio.openWritingPipe(write_addr);
			radio.setPALevel(RF24_PA_MIN);
			states[1] = 1;
			break;
		case 1: // STATE 1 | SEND
			if (!outgoingQueue.isEmpty()) {
				NRFMessage output = outgoingQueue.dequeue();
				Serial.print("Sending: ");
				Serial.println(output.msg);
				radio.stopListening();
				radio.write(&(output.msg), sizeof(output.len));
				radio.startListening();
			}
			break;
	}

	// TASK 2 | NRF RECEIVE
	switch (states[2]) {
		case -1: // STATE E-STOP
			states[2] = 1;
			break;
		case 0: // STATE 0 | INIT
			radio.openReadingPipe(1, read_addr);
			radio.startListening();
			states[2] = 1;
			break;
		case 1: // STATE 1 | RECEIVE
			if (radio.available()) {
				char input[32] = "";
				radio.read(&input, sizeof(input));
				incomingQueue.enqueue(NRFMessage(&input[0], 32)); // I don't think we really need an incoming queue, we should just deal with messages as we get them
				radio.flush_tx();

				// TODO: a lot

				// CHECKPOINT
				if (!memcmp(&input[5], &"PNT", 3)) {
					unsigned int temp = ((unsigned int) input[16]) >> 1;
					if (temp < CHECKPOINT_NUMBER) {
						checkpoints[temp].set((short) input[0]);
					}
				}

			}
			break;

			// CHECKPOINT -> set(vehicleId)
	}

	// TASK 3 | TRACK BLOCK 0
	switch (states[3]) {
		case -1: // STATE E-STOP
			track[0].engage(); // Engage brakes
			// If E-Stop is unpressed and reset is pressed:
			if (expander.digitalRead(PIN_ESTOP_BUTTON) == BUTTON_UNPRESSED
				&& expander.digitalRead(PIN_RESET_BUTTON) == BUTTON_PRESSED
			) {
				states[3] = 1; // Move to State 1
			}
			if (expander.digitalRead(PIN_RESET_BUTTON) == BUTTON_PRESSED
				&& expander.digitalRead(PIN_THEMING_BUTTON) == BUTTON_PRESSED
				&& expander.digitalRead(PIN_BRAKE0_SWITCH) == SWITCH_ON
				&& expander.digitalRead(PIN_BRAKE0_SWITCH) != lastStateSwitch0
			) {
				track[0].disengage();
				states[3] = 2;
			}
			lastStateSwitch0 = expander.digitalRead(PIN_BRAKE0_SWITCH);
			break;
		case 0: // STATE 0 | INIT
			// Configure the section
			track[0].configure(
				PIN_BRAKE0_SERVO,
				SBRAKE0_ENGAGED,
				SBRAKE0_DISENGAGED,
				SECTION0_END,
				SECTION0_BRAKE,
				&track[1]
			);
			states[3] = 1; // Move to State 1
			break;
		case 1: // STATE 1 | STANDARD OPERATIONS
			// If manual override switch is flipped:
			if (expander.digitalRead(PIN_BRAKE0_SWITCH) == SWITCH_ON) {
				track[0].engage(); // Engage brakes
				states[3] = 2; // Move to State 2
				break;
			}
			// If vehicle passes brake point and no vehicle is in next section:
			if (track[0].getBrake()->test() && !track[0].next()->vehicle()) {
				track[0].disengage(); // Disengage brakes;
				track[0].getBrake()->clear(); // Clear brake point;
			}
			// If vehicle passes end point:
			if (track[0].getEnd()->test()) {
				track[0].next()->setVehicle(track[0].vehicle()); // Put vehicle to next section
				track[0].clear(); // Clear section
				track[0].engage(); // Engage brakes
			}
			break;
		case 2: // STATE 2 | MANUAL OVERRIDE
			// If manual override switch is flipped back:
			if (expander.digitalRead(PIN_BRAKE0_SWITCH) == SWITCH_OFF) {
				track[0].engage();
				states[3] = 1; // Move to State 1
			}
			break;
	}

	// TASK 4 | TRACK BLOCK 1
	switch (states[4]) {
		case -1: // STATE E-STOP
			track[1].engage(); // Engage brakes
			// If E-Stop is unpressed and reset is pressed:
			if (expander.digitalRead(PIN_ESTOP_BUTTON) == BUTTON_UNPRESSED
				&& expander.digitalRead(PIN_RESET_BUTTON) == BUTTON_PRESSED
			) {
				states[4] = 1; // Move to State 1
			}
			if (expander.digitalRead(PIN_RESET_BUTTON) == BUTTON_PRESSED
				&& expander.digitalRead(PIN_THEMING_BUTTON) == BUTTON_PRESSED
				&& expander.digitalRead(PIN_BRAKE1_SWITCH) == SWITCH_ON
				&& expander.digitalRead(PIN_BRAKE1_SWITCH) != lastStateSwitch1
			) {
				track[1].disengage();
				states[4] = 2;
			}
			lastStateSwitch1 = expander.digitalRead(PIN_BRAKE1_SWITCH);
			break;
		case 0: // STATE 0 | INIT
			// Configure the section
			track[1].configure(
				PIN_BRAKE1_SERVO,
				SBRAKE1_ENGAGED,
				SBRAKE1_DISENGAGED,
				SECTION1_END,
				SECTION1_BRAKE,
				&track[2]
			);
			states[4] = 1; // Move to State 1
			break;
		case 1: // STATE 1 | STANDARD OPERATIONS
			// If manual override switch is flipped:
			if (expander.digitalRead(PIN_BRAKE1_SWITCH) == SWITCH_ON) {
				track[1].engage(); // Engage brakes
				states[4] = 2; // Move to State 2
				break;
			}
			// If vehicle passes brake point and no vehicle is in next section:
			if (track[1].getBrake()->test() && !track[1].next()->vehicle()) {
				track[1].disengage(); // Disengage brakes;
				track[1].getBrake()->clear(); // Clear brake point;
			}
			// If vehicle passes end point:
			if (track[1].getEnd()->test()) {
				track[1].next()->setVehicle(track[1].vehicle()); // Put vehicle to next section
				track[1].clear(); // Clear section
				track[1].engage(); // Engage brakes
			}
			break;
		case 2: // STATE 2 | MANUAL OVERRIDE
			// If manual override switch is flipped back:
			if (expander.digitalRead(PIN_BRAKE1_SWITCH) == SWITCH_OFF) {
				track[1].engage();
				states[4] = 1; // Move to State 1
			}
			break;
	}

	// TASK 5 | TRACK BLOCK 2
	switch (states[5]) {
		case -1: // STATE E-STOP
			track[2].engage(); // Engage brakes
			// If E-Stop is unpressed and reset is pressed:
			if (expander.digitalRead(PIN_ESTOP_BUTTON) == BUTTON_UNPRESSED
				&& expander.digitalRead(PIN_RESET_BUTTON) == BUTTON_PRESSED
			) {
				states[5] = 1; // Move to State 1
			}
			if (expander.digitalRead(PIN_RESET_BUTTON) == BUTTON_PRESSED
				&& expander.digitalRead(PIN_THEMING_BUTTON) == BUTTON_PRESSED
				&& expander.digitalRead(PIN_BRAKE2_SWITCH) == SWITCH_ON
				&& expander.digitalRead(PIN_BRAKE2_SWITCH) != lastStateSwitch2
			) {
				track[2].disengage();
				states[5] = 2;
			}
			lastStateSwitch2 = expander.digitalRead(PIN_BRAKE2_SWITCH);
			break;
		case 0: // STATE 0 | INIT
			// Configure the section
			track[2].configure(
				PIN_BRAKE2_SERVO,
				SBRAKE2_ENGAGED,
				SBRAKE2_DISENGAGED,
				SECTION2_END,
				SECTION2_BRAKE,
				&track[3]
			);
			states[5] = 1; // Move to State 1
			break;
		case 1: // STATE 1 | STANDARD OPERATIONS
			// If manual override switch is flipped:
			if (expander.digitalRead(PIN_BRAKE2_SWITCH) == SWITCH_ON) {
				track[2].engage(); // Engage brakes
				states[5] = 2; // Move to State 2
				break;
			}
			// If vehicle passes brake point and no vehicle is in next section:
			if (track[2].getBrake()->test() && !track[2].next()->vehicle()) {
				track[2].disengage(); // Disengage brakes;
				track[2].getBrake()->clear(); // Clear brake point;
			}
			// If vehicle passes end point:
			if (track[2].getEnd()->test()) {
				track[2].next()->setVehicle(track[2].vehicle()); // Put vehicle to next section
				track[2].clear(); // Clear section
				track[2].engage(); // Engage brakes
			}
			break;
		case 2: // STATE 2 | MANUAL OVERRIDE
			// If manual override switch is flipped back:
			if (expander.digitalRead(PIN_BRAKE2_SWITCH) == SWITCH_OFF) {
				track[2].engage();
				states[5] = 1; // Move to State 1
			}
			break;
	}

	// TASK 6 | TRACK BLOCK 3
	switch (states[6]) {
		case -1: // STATE E-STOP
			track[3].engage(); // Engage brakes
			// If E-Stop is unpressed and reset is pressed:
			if (expander.digitalRead(PIN_ESTOP_BUTTON) == BUTTON_UNPRESSED
				&& expander.digitalRead(PIN_RESET_BUTTON) == BUTTON_PRESSED
			) {
				states[6] = 1; // Move to State 1
			}
			if (expander.digitalRead(PIN_RESET_BUTTON) == BUTTON_PRESSED
				&& expander.digitalRead(PIN_THEMING_BUTTON) == BUTTON_PRESSED
				&& expander.digitalRead(PIN_START_BUTTON) == BUTTON_PRESSED
				&& expander.digitalRead(PIN_START_BUTTON) != lastStateStart
			) {
				track[3].disengage();
				states[6] = 2;
			}
			lastStateStart = expander.digitalRead(PIN_START_BUTTON);
			break;
		case 0: // STATE 0 | INIT
			// Configure the section
			track[3].configure(
				PIN_BRAKE3_SERVO,
				SBRAKE3_ENGAGED,
				SBRAKE3_DISENGAGED,
				SECTION3_END,
				SECTION3_BRAKE,
				&track[0]
			);
			states[6] = 1; // Move to State 1
			break;
		case 1: // STATE 1 | STANDARD OPERATIONS
			// If start button held:
			if (expander.digitalRead(PIN_START_BUTTON) == BUTTON_PRESSED
				&& expander.digitalRead(PIN_START_BUTTON)!= lastStateStart
			) {
				startHoldDown = millis();
			}
			if (expander.digitalRead(PIN_START_BUTTON) == BUTTON_PRESSED
				&& millis() - startHoldDown > LONG_PRESS_DURATION
			) {
				track[3].disengage(); // Disengage brakes
				states[6] = 2; // Move to State 2
				break;
			}
			lastStateStart = expander.digitalRead(PIN_START_BUTTON);
			// If start button pressed and no vehicle is in next section:
			if (expander.digitalRead(PIN_START_BUTTON) && !track[3].next()->vehicle()) {
				track[3].disengage(); // Disengage brakes;
			}
			// If vehicle passes end point:
			if (track[3].getEnd()->test()) {
				track[3].next()->setVehicle(track[3].vehicle()); // Put vehicle to next section
				track[3].clear(); // Clear section
				track[3].engage(); // Engage brakes
			}
			break;
		case 2: // STATE 2 | MANUAL OVERRIDE
			// If manual override switch is flipped back:
			if (expander.digitalRead(PIN_START_BUTTON) == BUTTON_PRESSED
				&& expander.digitalRead(PIN_START_BUTTON)!= lastStateStart
			) {
				track[3].engage();
				states[6] = 1; // Move to State 1
			}
			lastStateStart = expander.digitalRead(PIN_START_BUTTON);
			// If vehicle passes end point:
			if (track[3].getEnd()->test()) {
				track[3].next()->setVehicle(track[3].vehicle()); // Put vehicle to next section
				track[3].clear(); // Clear section
			}
			break;
	}

	// TASK 7 | SERIAL WRITE
	switch (states[7]) {
		case -1: // STATE E-STOP
			break;
		case 0: // STATE 0 | INIT
			Serial.begin(BAUD_RATE);
			break;
		case 1: // STATE 1 | WRITE
			break;
	}

	// TASK 8 | THEMING
	switch (states[8]) {
		case -1: // STATE E-STOP
			Theming.write(0);
			// If E-Stop is unpressed and reset is pressed:
			if (expander.digitalRead(PIN_ESTOP_BUTTON) == BUTTON_UNPRESSED
				&& expander.digitalRead(PIN_RESET_BUTTON) == BUTTON_PRESSED
			) {
				Theming.write(THEMING_SPEED);
				states[8] = 1;
			}
			break;
		case 0: // STATE 0 | INIT
			Theming.attach(PIN_THEMING_SERVO);
			Theming.write(THEMING_SPEED);
			states[8] = 1;
			lastStateTheming = expander.digitalRead(PIN_THEMING_BUTTON);
			break;
		case 1: // STATE 1 | RUNNING
			if (expander.digitalRead(PIN_THEMING_BUTTON) == BUTTON_PRESSED
				&& lastStateTheming != expander.digitalRead(PIN_THEMING_BUTTON)
			) {
				Theming.write(0);
				states[8] = 2;
			}
			lastStateTheming = expander.digitalRead(PIN_THEMING_BUTTON);
			break;
		case 2: // STATE 2 | STOPPED
			if (expander.digitalRead(PIN_THEMING_BUTTON) == BUTTON_PRESSED
				&& lastStateTheming != expander.digitalRead(PIN_THEMING_BUTTON)
			) {
				Theming.write(THEMING_SPEED);
				states[8] = 1;
			}
			lastStateTheming = expander.digitalRead(PIN_THEMING_BUTTON);
			break;
	}
}

void estop_interrupt() {
	for (int i = 0; i < sizeof(states)/sizeof(states[0]); i++) {
		states[i] = -1;
	}
	return;
}

char* createString(unsigned long msg_id, byte vehicle_id, char* type, byte* data, int length) {
	char* s = malloc(32);
	memcpy(s, &(msg_id) + 12, 4);
	memcpy(s + 4, &vehicle_id, 1);
	memcpy(s + 5, type, 3);
	memcpy(s + 8, data, length);
	return s;
}