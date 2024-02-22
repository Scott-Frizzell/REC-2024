#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <ArduinoQueue.h>
#include "NRFMessage.h"
#include <Servo.h>
#include "TrackSection.h"

#define BAUD_RATE 9600
#define BUTTON_PRESSED HIGH
#define BUTTON_UNPRESSED LOW
#define SWITCH_ON HIGH
#define SWITCH_OFF LOW
#define LED_ENABLED LOW
#define LED_DISABLED HIGH
#define PIN_CE 7
#define PIN_CSN 8
#define PIN_BRAKE1_SERVO 1
#define PIN_BRAKE2_SERVO 2
#define PIN_BRAKE3_SERVO 3
#define PIN_START_BUTTON 4
#define PIN_ESTOP_BUTTON 5
#define PIN_KEY_SWITCH 6
#define PIN_BRAKE1_SWITCH 9
#define PIN_BRAKE2_SWITCH 10
#define PIN_BRAKE3_SWITCH 11
#define PIN_POWER_INDICATOR 12
#define PIN_RESET_BUTTON 13
#define SBRAKE1_ENGAGED 0
#define SBRAKE1_DISENGAGED 90
#define SBRAKE2_ENGAGED 0
#define SBRAKE2_DISENGAGED 90
#define SBRAKE3_ENGAGED 0
#define SBRAKE3_DISENGAGED 90
#define SECTION_NUMBER 3
#define CHECKPOINT_NUMBER 20
#define SECTION1_START 0
#define SECTION1_BRAKE 1
#define SECTION1_END 4

int states[9];
ArduinoQueue<NRFMessage> outgoingQueue = ArduinoQueue<NRFMessage>(10);
ArduinoQueue<NRFMessage> incomingQueue = ArduinoQueue<NRFMessage>(10);
RF24 radio(PIN_CE, PIN_CSN);
const byte write_addr[6] = "00001";
const byte read_addr[6] = "00002";
const Servo SBrake1;
const Servo SBrake2;
const Servo SBrake3;
const TrackSection track[SECTION_NUMBER];
const Checkpoint checkpoints[CHECKPOINT_NUMBER];

void setup() {
	radio.begin();
}

void loop() {
	// TASK 0 | GENERAL RIDE FLOW
	switch (states[0]) {
		case -1: // STATE E-STOP
			break;
		case 0: // STATE 0 | INIT
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
				if (!memcmp(&input[8], &"CHECKPNT", 8)) {
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
	TrackSection section = track[0];
	int state = states[3];
	switch (state) {

		case -1: // STATE E-STOP

			section.engage(); // Engage brakes

			// If E-Stop is unpressed and reset is pressed:
			if (digitalRead(PIN_ESTOP_BUTTON) == BUTTON_UNPRESSED && digitalRead(PIN_RESET_BUTTON) == BUTTON_PRESSED) {
				
				state = 1; // Move to State 1

			}

			break;
			
		case 0: // STATE 0 | INIT

			// Configure the section
			section.configure(
				PIN_BRAKE1_SERVO,
				SBRAKE1_ENGAGED,
				SBRAKE1_DISENGAGED,
				SECTION1_START,
				SECTION1_END,
				SECTION1_BRAKE,
				&track[1]
			);

			states[3] = 1; // Move to State 1

			break;

		case 1: // STATE 1 | STANDARD OPERATIONS

			// If manual override switch is flipped:
			if (digitalRead(PIN_BRAKE1_SWITCH) == SWITCH_ON) {

				section.engage(); // Engage brakes

				state = 2; // Move to State 2

				break;

			}

			// If vehicle is waiting, and no vehicle is in the section, and the next section is not waiting:
			if (section.waiting() && !section.vehicle() && !section.next()->waiting()) {

				section.disengage(); // Disengage brakes

				// TODO: Add failsafe re-engage if brake clear is missed
			}

			// If vehicle passes start checkpoint:
			if (section.getStart()->test()) {

				// This means 2 cars crashed into each other...
				if (section.waiting()) {

					estop_interrupt(); // So trigger an E-Stop (This should never happen)

					break;

				}
				
				section.wait(section.getStart()->clear()); // Set that vehicle as waiting

			}

			// If vehicle passes brake clear checkpoint:
			if (section.getBrakeClear()->clear()) {

				section.engage(); // Re-engage brakes

			}

			// If vehicle passes end checkpoint:
			if (section.getEnd()->clear()) {

				section.clear(); // Clear the section

			}

			break;

		case 2: // STATE 2 | MANUAL OVERRIDE

			// If manual override switch is flipped back:
			if (digitalRead(PIN_BRAKE1_SWITCH) == SWITCH_OFF) {
				
				state = 1; // Move to State 1

			}

			break;
	}

	// TASK 4 | BRAKE RUN 2
	switch (states[4]) {
		case -1: // STATE E-STOP
			SBrake2.write(SBRAKE2_ENGAGED);
			if (digitalRead(PIN_ESTOP_BUTTON) == BUTTON_UNPRESSED && digitalRead(PIN_RESET_BUTTON) == BUTTON_PRESSED) {
				states[4] = 1;
			}
			break;
		case 0: // STATE 0 | INIT
			SBrake2.attach(PIN_BRAKE2_SERVO);
			SBrake2.write(SBRAKE2_ENGAGED);
			states[4] = 1;
			break;
		case 1: // STATE 1 | STANDARD OPERATIONS
			if (digitalRead(PIN_BRAKE2_SWITCH) == SWITCH_ON) {
				SBrake2.write(SBRAKE2_DISENGAGED);
				states[4] = 2;
			}
			break;
		case 2: // STATE 2 | MANUAL OVERRIDE
			if (digitalRead(PIN_BRAKE2_SWITCH) == SWITCH_OFF) {
				SBrake2.write(SBRAKE2_ENGAGED);
				states[4] = 1;
			}
			break;
	}

	// TASK 5 | BRAKE RUN 3
	switch (states[5]) {
		case -1: // STATE E-STOP
			SBrake3.write(SBRAKE3_ENGAGED);
			if (digitalRead(PIN_ESTOP_BUTTON) == BUTTON_UNPRESSED && digitalRead(PIN_RESET_BUTTON) == BUTTON_PRESSED) {
				states[5] = 1;
			}
			break;
		case 0: // STATE 0 | INIT
			SBrake3.attach(PIN_BRAKE3_SERVO);
			SBrake3.write(SBRAKE3_ENGAGED);
			states[5] = 1;
			break;
		case 1: // STATE 1 | STANDARD OPERATIONS
			if (digitalRead(PIN_BRAKE3_SWITCH) == SWITCH_ON) {
				SBrake3.write(SBRAKE3_DISENGAGED);
				states[5] = 2;
			}
			break;
		case 2: // STATE 2 | MANUAL OVERRIDE
			if (digitalRead(PIN_BRAKE3_SWITCH) == SWITCH_OFF) {
				SBrake3.write(SBRAKE3_ENGAGED);
				states[5] = 1;
			}
			break;
	}

	// TASK 6 | SERIAL WRITE
	switch (states[6]) {
		case -1: // STATE E-STOP
			break;
		case 0: // STATE 0 | INIT
			Serial.begin(BAUD_RATE);
			break;
		case 1: // STATE 1 | WRITE
			break;
	}

	// TASK 7 | THEMING
	switch (states[7]) {
		case -1: // STATE E-STOP
			break;
		case 0: // STATE 0 | INIT
			break;
		case 1: // STATE 1 | STOPPED
			break;
		case 2: // STATE 2 | RUNNING
			break;
	}

	// TASK 8 | OPERATOR INPUT
	switch (states[8]) {
		case -1: // STATE E-STOP
			break;
		case 0: // STATE 0 | INIT
		digitalWrite(PIN_POWER_INDICATOR, LED_ENABLED);
		states[8] = 1;
			break;
		case 1: // STATE 1 | REGULAR OPERATION
			break;
	}
}

void estop_interrupt() {
	for (int i = 0; i < sizeof(states)/sizeof(states[0]); i++) {
		states[i] = -1;
	}
	return;
}
