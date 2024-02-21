#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <ArduinoQueue.h>
#include "NRFMessage.h"

RF24 radio(7, 8);

ArduinoQueue<NRFMessage> outgoingQueue = ArduinoQueue<NRFMessage>(10);
ArduinoQueue<NRFMessage> incomingQueue = ArduinoQueue<NRFMessage>(10);

const byte write_addr[6] = "00001";
const byte read_addr[6] = "00002";

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openWritingPipe(write_addr);
  radio.openReadingPipe(1, read_addr);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    char input[32] = "";
    radio.read(&input, sizeof(input));
    incomingQueue.enqueue(NRFMessage(&input[0], 32));
    radio.flush_tx();
  }
  if (!incomingQueue.isEmpty()) {
    NRFMessage temp = incomingQueue.dequeue();
    Serial.print("Received: ");
    Serial.println(temp.msg);
  }
  outgoingQueue.enqueue(NRFMessage("Ping", sizeof("Ping")));
  if (!outgoingQueue.isEmpty()) {
    radio.stopListening();
    NRFMessage output = outgoingQueue.dequeue();
    Serial.print("Sending: ");
    Serial.println(output.msg);
    radio.write(&(output.msg), sizeof(output.len));
    radio.startListening();
  }
  delay(1000);
}
