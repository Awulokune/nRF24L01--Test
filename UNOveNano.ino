
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

RF24 radio(9, 10);

const byte address1[6] = "AWU01";
const byte address2[6] = "AWU02";

#ifdef ARDUINO_AVR_NANO
#define DEVICE_ID "NANO"
Adafruit_SSD1306 display(128, 64, &Wire, -1);
#else
#define DEVICE_ID "UNO"
#endif

struct Packet {
  char sender[8];
  unsigned long counter;
};

unsigned long lastSend = 0;
unsigned long counter = 0;

void setup() {
  Serial.begin(9600);

#ifdef ARDUINO_AVR_NANO
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("NANO RX/TX");
  display.println("Bekleniyor...");
  display.display();
#endif

  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.setAutoAck(true);

#ifdef ARDUINO_AVR_NANO
  radio.openWritingPipe(address2);
  radio.openReadingPipe(1, address1);
#else
  radio.openWritingPipe(address1);
  radio.openReadingPipe(1, address2);
#endif

  radio.startListening();
}

void loop() {

  // Gelen verileri dinle
  if (radio.available()) {
    Packet packet;
    radio.read(&packet, sizeof(packet));

    Serial.print("GELDI: ");
    Serial.print(packet.sender);
    Serial.print(" - ");
    Serial.println(packet.counter);

#ifdef ARDUINO_AVR_NANO
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 10);
    display.println("VERI");
    display.println("GELDI!");
    display.setTextSize(1);
    display.setCursor(0, 52);
    display.print(packet.sender);
    display.print(" #");
    display.print(packet.counter);
    display.display();
#endif
  }

  // Her saniye veri gönder
  if (millis() - lastSend >= 1000) {
    lastSend = millis();

    Packet packet;
    strncpy(packet.sender, DEVICE_ID, sizeof(packet.sender));
    packet.sender[sizeof(packet.sender) - 1] = '\0';
    packet.counter = ++counter;

    radio.stopListening();
    bool ok = radio.write(&packet, sizeof(packet));
    radio.startListening();

    Serial.println(ok ? "TX BASARILI" : "TX BASARISIZ");
  }
}
