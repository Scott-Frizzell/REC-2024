#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <ArduinoQueue.h>
#include "NRFMessage.h"

RF24 radio(7, 8);

ArduinoQueue<NRFMessage> outgoingQueue = ArduinoQueue<NRFMessage>(10);
ArduinoQueue<NRFMessage> incomingQueue = ArduinoQueue<NRFMessage>(10);

const byte writeaddr[6] = "00002";
const byte readaddr[6] = "00001";

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openWritingPipe(writeaddr);
  radio.openReadingPipe(1, readaddr);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    char input[32] = "";
    radio.read(&input, sizeof(input));
    NRFMessage temp = NRFMessage(&(input[0]), 32);
    Serial.print("Received: ");
    Serial.println(temp.msg);
    if (!memcmp(&(temp.msg[0]), "Ping", sizeof("Ping"))) {
      outgoingQueue.enqueue(NRFMessage("Pong", sizeof("Pong")));
    }
    radio.flush_tx();
  }
  if (!outgoingQueue.isEmpty()) {
      NRFMessage output = outgoingQueue.dequeue();
      Serial.print("Sending: ");
      Serial.println(output.msg);
      radio.stopListening();
      radio.write(&(output.msg), output.len);
      radio.startListening();
  }
  delay(1000);
}
