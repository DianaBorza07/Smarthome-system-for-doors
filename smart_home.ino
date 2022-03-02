#include <Servo.h>
#include <SoftwareSerial.h>
SoftwareSerial Bluetooth(10, 9); // RX 9, TX 10
int sensorPIR = 7;
int ledPinRed = 4; // for open/close door
int ledPinBlue = 5; // for distance
int inputValue;
int servoPos ;
char data;
Servo servo;
int state = LOW;
long startTime ;
long delayValue;
int flag = 0;
int timeStamp;
int echoPin = 28;
int trigPin = 29;
long duration;
int distance;

void setup() {

  setupTime();
  pinMode(ledPinRed, OUTPUT);
  pinMode(ledPinBlue, OUTPUT);
  pinMode(sensorPIR, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(9600);
  Bluetooth.begin(9600);
  servo.attach(3);
  servo.write(0);
  initDoorPosition();

}


void loop() {

  if (!Serial.available() && flag != 1) {

    inputValue = digitalRead(sensorPIR);  // read sensor input
    Serial.print("Sensor PIR input value: ");
    Serial.println(inputValue);
    digitalWrite(ledPinBlue, LOW);
    if (inputValue == HIGH) {
      getDistance();
      triggerDistance();
      delay(400);

      if (state == LOW && distance <= 30 ) { // input value = high and previous state = low -> motion detected
        digitalWrite(ledPinRed, HIGH); // turn led on
        state = HIGH;
        Serial.println("Motion detected");
        open_door();
        startTime = millis();
      }
    }
    else {
      delay(1000);

      if (state == HIGH) { // input value = low and previous state = high -> motion stopped
        state = LOW;
        Serial.println("Motion stopped");
        delayValue = millis() - startTime;
        while (delayValue < 5000)
        { delay(1000);
          delayValue = millis() - startTime;
        }

        digitalWrite(ledPinRed, LOW); // turn led off
        close_door();

      }
    }
  }

  if (Bluetooth.available()) {
    data = Bluetooth.read();
    if (data == '1' && state == LOW) {
      Serial.println("Door opened from phone");
      digitalWrite(ledPinRed, HIGH);
      open_door();
      flag = 1; // read from bluetooth
      state = HIGH;
      startTime = millis();
    }
    else {
      if (data == '0' && state == HIGH)
      {
        Serial.println("Door closed from phone");
        digitalWrite(ledPinRed, LOW);
        close_door();
        flag = 0;
        state = LOW;
      }
    }

    while (flag == 1 && millis() - startTime < 5000)
    { timeStamp++;
      delay(1000);
    }

    if (timeStamp >= 5 && flag == 1) {
      Serial.println("Automatic close door");
      digitalWrite(ledPinRed, LOW);
      close_door();
      timeStamp = 0;
      flag = 0;
      state = LOW;
    }
  }

}

void open_door() {
  int currentPos = servo.read();
  for (servoPos = currentPos; servoPos <= 90; servoPos++) { // open door
    servo.write(servoPos);
    blinkLed();
  }
}

void close_door() {
  int currentPos = servo.read();
  for (servoPos = currentPos; servoPos >= 0; servoPos--) { //close door
    servo.write(servoPos);
    delay(50);
  }
}


void setupTime() {
  delay(4000);
}

int getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;
  Serial.print("Distance: ");
  Serial.println(distance);
}

void blinkLed() { // blink led blue while open door => distance <=30
  digitalWrite(ledPinBlue, LOW);
  delay(50);
  digitalWrite(ledPinBlue, HIGH);
  delay(50);
  digitalWrite(ledPinBlue, LOW);
}

void triggerDistance() { // turn blue led on if distance < 200 and distance >30
  Serial.print("Distance: ");
  Serial.println(distance);
  if (distance < 200 && distance > 30)
    digitalWrite(ledPinBlue, HIGH);
  else
    digitalWrite(ledPinBlue, LOW);
}

void initDoorPosition() {
  int currentPos = servo.read();
  if (currentPos > 5) {
    for (int pos = currentPos; pos >= 0; pos--) {
      servo.write(pos);
      delay(50);
    }
  }
}
