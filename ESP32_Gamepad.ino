 #include <BleGamepad.h>

#define VRX_PIN 34
#define VRY_PIN 35
#define SW_PIN  32

// Keep these pins matched to your physical wiring.
const int BUTTON_PINS[4] = {12, 14, 27, 26};

const int DEADZONE = 120;

BleGamepad bleGamepad("ESP32 Custom Pad", "DIY", 100);

int centerX = 2048;
int centerY = 2048;

int findCenter(int pin) {
  long total = 0;

  // Average readings while the joystick is resting in the center.
  for (int i = 0; i < 50; i++) {
    total += analogRead(pin);
    delay(5);
  }

  return total / 50;
}

int16_t readAxis(int pin, int center) {
  int raw = analogRead(pin);
  int delta = raw - center;
  long value = 0;

  if (delta > DEADZONE) {
    value = map(delta, DEADZONE, 4095 - center, 0, 32767);
  } else if (delta < -DEADZONE) {
    value = map(delta, -center, -DEADZONE, -32767, 0);
  }

  value = constrain(value, -32767, 32767);
  return (int16_t)value;
}

void setup() {
  Serial.begin(115200);

  pinMode(SW_PIN, INPUT_PULLUP);

  for (int i = 0; i < 4; i++) {
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
  }

  // Leave the joystick centered during this calibration.
  centerX = findCenter(VRX_PIN);
  centerY = findCenter(VRY_PIN);

  Serial.print("Joystick center X: ");
  Serial.println(centerX);
  Serial.print("Joystick center Y: ");
  Serial.println(centerY);

  Serial.println("Starting BLE Gamepad...");
  bleGamepad.begin();
}

void loop() {
  if (!bleGamepad.isConnected()) {
    delay(100);
    return;
  }

  int16_t x = readAxis(VRX_PIN, centerX);
  int16_t y = readAxis(VRY_PIN, centerY);

  bleGamepad.setLeftThumb(x, y);

  // Four wired buttons send gamepad buttons 1–4.
  for (int i = 0; i < 4; i++) {
    if (digitalRead(BUTTON_PINS[i]) == LOW) {
      bleGamepad.press(i + 1);
    } else {
      bleGamepad.release(i + 1);
    }
  }

  // Joystick click sends gamepad button 5.
  if (digitalRead(SW_PIN) == LOW) {
    bleGamepad.press(5);
  } else {
    bleGamepad.release(5);
  }

  bleGamepad.sendReport();
  delay(10);
}