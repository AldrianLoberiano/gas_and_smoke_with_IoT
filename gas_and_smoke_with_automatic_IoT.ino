#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C LCD address

// Pin definitions
#define MQ2pin A0
#define greenLED 4
#define yellowLED 3
#define redLED 2
#define buzzer 10

#define RL 5.0         // Load resistance in kΩ
float Ro = 10.0;       // Sensor resistance in clean air (adjust if calibrated)

void setup() {
  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(buzzer, OUTPUT);

  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(" FIRE AND SMOKE");
  delay(1000);
  lcd.setCursor(0, 1);
  lcd.print("DETECTION SYSTEM");
  delay(3000);
  lcd.clear();

  Serial.begin(9600);
}

// Convert raw ADC to sensor resistance (Rs)
float getSensorResistance(int raw_adc) {
  float voltage = (float)raw_adc * (5.0 / 1023.0);
  float resistance = (5.0 - voltage) / voltage * RL;
  return resistance;
}

// Estimate PPM from Rs/Ro ratio using MQ-2 log-log curve equation
float getPPM(float rs_ro_ratio) {
  float ppm_log = (log10(rs_ro_ratio) - 0.18) / -0.42;
  return pow(10, ppm_log);
}

void loop() {
  int sensorADC = analogRead(MQ2pin);
  float sensorResistance = getSensorResistance(sensorADC);
  float rs_ro_ratio = sensorResistance / Ro;
  float ppm = getPPM(rs_ro_ratio);

  // Display values on Serial Monitor
  Serial.print("ADC: ");
  Serial.print(sensorADC);
  Serial.print(" | Rs/Ro: ");
  Serial.print(rs_ro_ratio);
  Serial.print(" | PPM: ");
  Serial.print(ppm, 1);

  // Display on LCD
  lcd.setCursor(0, 0);
  lcd.print("VALUE: ");
  lcd.print(ppm, 1);
  lcd.print("ppm   ");  // Clear any leftover digits

  lcd.setCursor(0, 1);

  if (ppm < 50) {
    lcd.print("SMOKE LEVEL: LOW ");
    Serial.println(" | Status: LOW");
    digitalWrite(greenLED, HIGH);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, LOW);
    noTone(buzzer);
  } 
  else if (ppm >= 50 && ppm < 100) {
    lcd.print("SMOKE LEVEL: MED ");
    Serial.println(" | Status: MEDIUM");
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, HIGH);
    digitalWrite(redLED, LOW);
    noTone(buzzer);
  } 
  else {
    lcd.print("SMOKE LEVEL:HIGH");
    Serial.println(" | Status: HIGH");
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, HIGH);
    tone(buzzer, 1000);  // Continuous tone
  }

  delay(2000);
  lcd.clear();
}
