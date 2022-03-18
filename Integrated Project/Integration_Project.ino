#include <PubSubClient.h>
#include <MFRC522.h>
#include <IRremote.hpp>
#include <Adafruit_I2CDevice.h>
#include <LiquidCrystal_I2C.h>
#include <HCSR04.h>
#include "DHT.h"

// For DHT11
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// For HC-SR04
#define TRIG 13  // D7
#define ECHO 16  // D0
HCSR04 hc(TRIG, ECHO);

// For IR receiver
int RECV_PIN = 6;
#define code1 2534850111  // Press Button 1
#define code2 1033561079  // Press Button 2
#define code3 1635910171  // Press Button 3
IRrecv irrecv(RECV_PIN);
decode_results results;

// For Analog Joy Stick
#define joyX A0
#define joyY A1

// For RFID-RC522
#define RST_PIN 9               // Configurable, see typical pin layout above
#define SS_PIN 10               // Configurable, see typical pin layout above
MFRC522 rfid(SS_PIN, RST_PIN);  // Create MFRC522 instance.
MFRC522::MIFARE_Key key;
String tag;

// For I2c
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);  // (adress,column,row)

// Pins and Variable
const int ledPin = 7;
const int ledRed = 5;       //D1
const int ledGreen = 4;     //D2
const int ledBlue = 14;     //D5

int buttonPin = 12;         //D6
int buttonState = 0;
int lastState = 1;
int counter = 0;


// for send a message
#define MSG_BUFFER_SIZE (100)
char msg[MSG_BUFFER_SIZE];
char msgH[MSG_BUFFER_SIZE];
char msgT[MSG_BUFFER_SIZE];
char msgI[MSG_BUFFER_SIZE];

// Update these with values suitable for your network.
const char* ssid = "Panomporn 2g";
const char* password = "0812397357";
const char* mqtt_server = "broker.hivemq.com";

// connecting to wifi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// MQTT Client Publisher
WiFiClient espClient;
PubSubClient client(espClient);

// MQTT Client Subscriber
void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT
  if (topic == "room/led") {
    Serial.print("Changing Room Led to ");
    if (messageTemp == "on") {
      digitalWrite(ledPin, HIGH);
      Serial.print("On");
    } else if (messageTemp == "off") {
      digitalWrite(ledPin, LOW);
      Serial.print("Off");
    }
  }
  Serial.println();
}

// MQTT reconedted
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // add your subscriber
      client.subscribe("room/led");

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// this function runs once
void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(ledPin, OUTPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(ledBlue, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  lcd.init();               // Enable LCD 
  lcd.backlight();          // LCD Flashlight
  irrecv.enableIRIn();      // Enable IR
  SPI.begin();              // Master Slave Comunication
  rfid.PCD_Init();          // Enable RFID

  // For MQTT Communication
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

// loops
void loop() {

  lcd.clear();
  
  float distance = hc.dist();

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float hic = dht.computeHeatIndex(t, h, false);

  // MQTT ACCESS
  
  client.loop();

  // if disconect then reconected
  if (!client.connected()) {
    reconnect();
  }  
  // reconect with name: ESP8266Client
  if (!client.loop()) {
    client.connect("ESP8266Client");
  }
    

// RFID Access. 17616465131 for coin and 12821022043 for card
// other RFIDs tag can't access the code
if (! rfid.PICC_IsNewCardPresent())
  return;

if (rfid.PICC_ReadCardSerial()) {
  for (byte i = 0; i < 4; i++) {
  tag += rfid.uid.uidByte[i];
  }
  Serial.print("Tag:");
  Serial.println(tag);
  if (tag == "17616465131") {
    Serial.println("Access Granted!");
    digitalWrite(ledPin, HIGH);
    delay(1000);
    digitalWrite(ledPin, LOW);
    delay(1000);
  } else if(tag == "12821022043") {
    Serial.println("Access Granted!");
    digitalWrite(ledPin, HIGH);
    delay(1000);
    digitalWrite(ledPin, LOW);
    delay(1000);
  } else {
    Serial.println("Access Denied!");
    digitalWrite(ledPin, LOW);
  }
  tag = "";
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// Serial Monitor.
  
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  
  Serial.print(("Humidity: "));
  Serial.print(h);
  Serial.println(("%"));

  Serial.print(" Temperature: ");
  Serial.print(t);
  Serial.println(("°C "));

  Serial.print(("Heat index: "));
  Serial.print(hic);
  Serial.println(F("°C "));
  Serial.println();

  Serial.print("#counter: ");
  Serial.println(counter);

// LCD Print.
  lcd.print("Distance:");
  lcd.print(distance);
  lcd.print("cm.");

// Analog Joy Stick  
  float xValue = analogRead(joyX);
  float yValue = analogRead(joyY);
  Serial.print(xValue);
  Serial.print("\t");
  Serial.println(yValue);

// Control Led if humidity higher than 70 
// this LED can controlled by MQTT client
  if (h > 70)
    digitalWrite(ledPin, HIGH);
  else
    digitalWrite(ledPin, LOW);

// IR Receiver
  if (irrecv.decode(&results)) {
    Serial.print("Button Code: ");
    Serial.println(results.value);
    if (results.value == code1) {
      Serial.println("LED Red activate for 5 second");
      digitalWrite(ledRed, HIGH);
      lcd.clear();
      lcd.print("LED RED 3 Sec");
      delay(3000);
      digitalWrite(ledRed, LOW);
    } else if (results.value == code2) {
      Serial.println("LED Green activate for 5 second");
      digitalWrite(ledGreen, HIGH);
      lcd.clear();
      lcd.print("LED Green 3 Sec");
      delay(3000);
      digitalWrite(ledGreen, LOW);
    } else if (results.value == code3) {
      Serial.println("LED Blue activate for 5 second");
      digitalWrite(ledBlue, HIGH);
      lcd.clear();
      lcd.print("LED Blue 3 Sec");
      delay(3000);
      digitalWrite(ledBlue, LOW);
    }
    irrecv.resume();  // Receive the next value
  }


// Button State Function Control RGB LED color with anolog write
  buttonState = digitalRead(buttonPin);
  if (buttonState != lastState) {
    if (buttonState != 0) {
      Serial.print("Release");
      counter++;
    } else {
      Serial.println("HolD");
    }
    delay(30);
  }
  lastState = buttonState;

  lcd.setCursor(6, 1);
  if (counter % 4 == 0) {
    analogWrite(ledRed, 255 - distance);
    analogWrite(ledGreen, 0);
    analogWrite(ledBlue, 0);
    lcd.print("RED");
  } else if (counter % 4 == 1) {
    analogWrite(ledRed, 0);
    analogWrite(ledGreen, 255 - distance);
    analogWrite(ledBlue, 0);
    lcd.print("GREEN");
  } else if (counter % 4 == 2) {
    analogWrite(ledRed, 0);
    analogWrite(ledGreen, 0);
    analogWrite(ledBlue, 255 - distance);
    lcd.print("BLUE");
  } else {
    analogWrite(ledRed, 0);
    analogWrite(ledGreen, 0);
    analogWrite(ledBlue, 0);
    lcd.print("OFF");
  }
  delay(250);


// MQTT Publisher
  snprintf(msgR, MSG_BUFFER_SIZE, "255");
  snprintf(msgG, MSG_BUFFER_SIZE, "255");
  snprintf(msgB, MSG_BUFFER_SIZE, "255");
  client.publish("room/sensor/dht11/humidity", msgH);
  client.publish("room/sensor/dht11/temperature", msgT);
  client.publish("room/sensor/dht11/heatindex", msgH);


}