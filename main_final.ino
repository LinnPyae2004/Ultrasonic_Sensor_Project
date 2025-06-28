#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal.h>

Servo myServo;

// LCD wiring: (rs, en, d4, d5, d6, d7)
LiquidCrystal lcd(10, 9, 8, 7, 6, 5);

const int echo = 2;
const int trig = 3;
const int servoPin = 12;
const int buzzer = 11;
const int redLed = 4;

unsigned int distance = 0;
int currentAngle = 0;
int increment = 1;
bool isWarning = false;
int detectionCount = 0;
const int threshold = 20;
const int detectionTrigger = 3;
const unsigned long updateInterval = 20; // 20 ms
unsigned long lastServoUpdate = 0;

String lastLcdState = "";  // NEW: Track the last message displayed on the LCD

void setup() {
  Serial.begin(9600);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(redLed, OUTPUT);
  digitalWrite(trig, LOW);

  lcd.begin(16, 2);
  myServo.attach(servoPin);
  lcd.print("System Ready");
  delay(1000);
  lcd.clear();
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastServoUpdate >= updateInterval) {
    lastServoUpdate = currentMillis;

    // Sweep servo
    if (!isWarning) {
      myServo.write(currentAngle);
      currentAngle += increment;

      if (currentAngle >= 180) {
        currentAngle = 180;
        increment = -1;
      } else if (currentAngle <= 0) {
        currentAngle = 0;
        increment = 1;
      }
    }

    // Check distance every 1 degrees
    if (currentAngle % 1 == 0) {
      distance = getDistance();
      Serial.print("Distance: ");
      Serial.print(distance);
      Serial.println(" cm");

      if (distance < threshold) {
        detectionCount++;
      } else {
        detectionCount = 0;
      }

      if (detectionCount >= detectionTrigger && !isWarning) {
        lcdSetWarning(); // an object detected
        isWarning = true;
      } else if (detectionCount == 0 && isWarning) {
        lcdSetEmpty(); // nothing detected
        isWarning = false;
      }
    }
  }
}

unsigned int getDistance() {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  unsigned long LowLevelTime = pulseInLong(echo, LOW);
  if (LowLevelTime >= 50000) {
    Serial.println("Invalid");
    return 999;
  }
  return LowLevelTime / 50;
}

void lcdSetWarning() {
  if (lastLcdState != "warning") {
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("WARNING!!");
    lcd.setCursor(2, 1);
    lcd.print("Foreign Body");
    lastLcdState = "warning";
  }
  digitalWrite(buzzer, HIGH);
  digitalWrite(redLed, HIGH);
}

void lcdSetEmpty() {
  if (lastLcdState != "empty") {
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Area is Empty");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lastLcdState = "empty";
  }
  digitalWrite(buzzer, LOW);
  digitalWrite(redLed, LOW);
}
