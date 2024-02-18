#include <RF24.h>
#include <nRF24L01.h>
#include <SPI.h>

RF24 radio(7, 8);

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
    Serial.print("Received: ");
    Serial.println(input);
    radio.flush_tx();
    if (!memcmp(&input, "Ping", sizeof("Ping"))) {
      radio.stopListening();
      char output[] = "Pong";
      Serial.print("Sending: ");
      Serial.println(output);
      radio.write(&output, sizeof(output));
      radio.startListening();
    }
  }
  delay(1000);
}
