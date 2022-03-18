// Date: 16 Mar 2022
// Intro to Embedded System Group 1
// LED fading by LDR

const int ledPin = 7;
const int buttonPin = 2;
const int ldrPin = A0;

int buttonState = 0;
int lastState = 1;
int counter = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ldrPin, INPUT);
}

void loop() {
  // button state pullup normally high (1)
  buttonState = digitalRead(buttonPin);

  if (buttonState != lastState) {
    if (buttonState == 1) {
      Serial.println("Release");
      counter++;
      Serial.print("counter: ");
      Serial.println(counter);
    } else {
      Serial.println("Hold");
    }
    // Delay a little bit to avoid bouncing
    delay(30);
  }
  // save the current state as the last state, for next time through the loop
  lastState = buttonState;

  // read intensity of light
  int sensorValue = analogRead(ldrPin);
  float voltage = sensorValue * (5.0 / 1023.0);

  // if #odd forward #even reverse
  if (counter % 2 == 1) {
    analogWrite(ledPin, voltage * 100);
  } else {
    analogWrite(ledPin, 255 - voltage * 150);
  }
  delay(500);
}
