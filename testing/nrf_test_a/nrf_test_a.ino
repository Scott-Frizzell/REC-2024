#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <cppQueue.h>

RF24 radio(7, 8);

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
  radio.stopListening();
  char output[] = "Ping";
  Serial.print("Sending: ");
  Serial.println(output);
  radio.write(&output, sizeof(output));
  radio.startListening();
  if (radio.available()) {
    char input[32] = "";
    radio.read(&input, sizeof(input));
    Serial.print("Received: ");
    Serial.println(input);
    radio.flush_tx();
  }
  delay(1000);
}
