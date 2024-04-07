// TODO: Raise EStop if too long between checkpoints

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <ArduinoQueue.h>
#include "NRFMessage.h"
#include <Servo.h>

#define BAUD_RATE 9600
#define PIN_CE 8
#define PIN_CSN 7
#define PIN_SERVO 4
#define PIN_HALL 2
#define PIN_BATTERY A0
#define SERVO_DEFAULT 0
#define CHECKPOINTS 10
#define HALL_DELAY 250

int states[4];
ArduinoQueue<NRFMessage> outgoingQueue = ArduinoQueue<NRFMessage>(10);
RF24 radio(PIN_CE, PIN_CSN);

const byte write_addr[6] = "00002";
const byte read_addr[6] = "00001";
const Servo servo;

int current_angle;
int target_angle = SERVO_DEFAULT;
int checkpoint = 0;
int angles[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
byte vehicle_id;
unsigned long armed;
int battery = 100;

void setup() {} // All setup is done in respective state 0's

void loop() {
	// TASK 0 | GENERAL RIDE FLOW
	switch (states[0]) {
		case -1: // STATE E-STOP
			if ((analogRead(PIN_BATTERY) - 2.5) * 200 != battery) {
				battery = (analogRead(PIN_BATTERY) - 2.5) * 200;
				outgoingQueue.enqueue(NRFMessage(createString(millis(), vehicle_id, "BAT", battery, 4), 32));
			}
			break;
		case 0: // STATE 0 | INIT
			Serial.begin(BAUD_RATE);
			pinMode(PIN_HALL, INPUT_PULLUP);
			pinMode(PIN_BATTERY, INPUT);
			attachInterrupt(digitalPinToInterrupt(PIN_HALL), hall_effect_interrupt, CHANGE);
			outgoingQueue.enqueue(NRFMessage(createString(millis(), vehicle_id, "NEW", 0, 0), 32));
			states[0] = 1;
			break;
		case 1: // STATE 1 | WAIT FOR CONNECTION
			if (vehicle_id == 0) {
				outgoingQueue.enqueue(NRFMessage(createString(millis(), vehicle_id, "NEW", 0, 0), 32));
			}
			break;
		case 2: // STATE 2 | NORMAL OPERATION
			if (target_angle != angles[checkpoint]) {
				target_angle = angles[checkpoint];
				outgoingQueue.enqueue(NRFMessage(createString(millis(), vehicle_id, "ANG", target_angle, 4), 32));
			}
			if ((analogRead(PIN_BATTERY) - 2.5) * 200 != battery) {
				battery = (analogRead(PIN_BATTERY) - 2.5) * 200;
				outgoingQueue.enqueue(NRFMessage(createString(millis(), vehicle_id, "BAT", battery, 4), 32));
			}
			break;
	}
	
	// TASK 1 | NRF SEND
	switch (states[1]) {
		case -1: // STATE E-STOP
            char msg[] = "ESTOP";
			Serial.print("Sending: ");
			Serial.println(msg);
			radio.stopListening();
			radio.write(&msg, sizeof(msg));
			radio.startListening();
            states[1] = 1;
			break;
		case 0: // STATE 0 | INIT
			radio.begin();
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
				char input[32];
				radio.read(&input, sizeof(input));
				if (!memcmp(&input[5], "STP", 3)) {
					// E-STOP
					estop_interrupt();
				} else if (!memcmp(&input[5], "PNG", 3)) {
					// PING
					// Do nothing, RF24 automatically acknowledges
				} else if (!memcmp(&input[5], "CLR", 3)) {
					// CLEAR E-STOP
					states[0] = 2;
					states[3] = 1;
				} else if (!memcmp(&input[5], "NEW", 3)) {
					// NEW VEHICLE
					if (!memcmp(&input[4], 0, 1)) {
						memcpy(&vehicle_id, &input[8], 1);
					}
				} else {
					// BAD MESSAGE
					// Do nothing
				}
				radio.flush_tx();
			}
			break;
	}

    // TASK 3 | TURNING CONTROL
	switch (states[3]) {
		case -1: // STATE E-STOP
			break;
		case 0: // STATE 0 | INIT
            servo.attach(PIN_SERVO);
            servo.write(SERVO_DEFAULT);
            states[3] = 1;
			break;
		case 1: // STATE 1 | NORMAL OPERATION
            if (current_angle != target_angle) {
                servo.write(target_angle);
				current_angle = target_angle;
            }
			break;
	}
}

void estop_interrupt() {
	for (int i = 0; i < sizeof(states)/sizeof(states[0]); i++) {
		states[i] = -1;
	}
	return;
}

void hall_effect_interrupt() {
	if (digitalRead(PIN_HALL) && millis() - armed > HALL_DELAY) {
		checkpoint = (checkpoint + 1) % CHECKPOINTS;
		armed = millis();
		outgoingQueue.enqueue(NRFMessage(createString(millis(), vehicle_id, "LOC", checkpoint, 4), 32));
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
