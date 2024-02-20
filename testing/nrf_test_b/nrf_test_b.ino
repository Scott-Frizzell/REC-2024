#include <RF24.h>
#include <nRF24L01.h>
#include <SPI.h>
#include <ArduinoQueue.h>

struct NRFMessage {
  char msg[32];
};

RF24 radio(7, 8);
ArduinoQueue<NRFMessage> outgoingQueue = ArduinoQueue<NRFMessage>(20);
ArduinoQueue<NRFMessage> incomingQueue = ArduinoQueue<NRFMessage>(20);

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
    struct NRFMessage temp = {input};
    incomingQueue.enqueue(&temp);
    radio.flush_tx();
  }
  if (!incomingQueue.isEmpty()) {
    char input[32] = {incomingQueue.dequeue()->msg};
    if (!memcmp(&input, "Ping", sizeof("Ping"))) {
      struct NRFMessage output = {"Pong"};
      outgoingQueue.enqueue(&output);
    }
  }
  if (!outgoingQueue.isEmpty()) {
      radio.stopListening();
      char output[32] = { outgoingQueue.dequeue()->msg};
      Serial.print("Sending: ");
      Serial.println(output);
      radio.write(&output, sizeof(output));
  }
  delay(1000);
}
