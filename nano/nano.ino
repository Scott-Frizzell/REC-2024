#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <ArduinoQueue.h>
#include "NRFMessage.h"
#include <Servo.h>

#define BAUD_RATE 9600
#define PIN_CE 7
#define PIN_CSN 8
#define PIN_SERVO 1
#define PIN_SOLENOID 2
#define PIN_HALL_POSITION 3
#define PIN_HALL_ANGLE 4
#define SERVO_DEFAULT 0
#define SOLENOID_ENGAGED LOW
#define SOLENOID_DISENGAGED HIGH
#define HALL_ANGLE_CENTERED HIGH

int states[4];
ArduinoQueue<NRFMessage> outgoingQueue = ArduinoQueue<NRFMessage>(10);
ArduinoQueue<NRFMessage> incomingQueue = ArduinoQueue<NRFMessage>(10);
RF24 radio(PIN_CE, PIN_CSN);
const byte write_addr[6] = "00002";
const byte read_addr[6] = "00001";
const Servo servo;
int current_position;
int target_position = SERVO_DEFAULT;

void setup() {
    Serial.begin(BAUD_RATE);
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
			radio.write(&msg, sizeof(msg));
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
            digitalWrite(PIN_SOLENOID, SOLENOID_DISENGAGED);
            states[3] = 2;
			break;
		case 1: // STATE 1 | STOPPED
            if (current_position !== target_position) {
                digitalWrite(PIN_SOLENOID, SOLENOID_DISENGAGED);
                servo.write(target_position);
                states[3] = 2;
            }
			break;
		case 2: // STATE 2 | RUNNING
            if (digitalRead(PIN_HALL_ANGLE) == HALL_ANGLE_CENTERED) {
                current_position = target_position;
                digitalWrite(PIN_SOLENOID, SOLENOID_ENGAGED);
                states[3] = 1;
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
    return;
}