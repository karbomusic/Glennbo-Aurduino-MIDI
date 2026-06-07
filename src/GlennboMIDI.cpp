#include <Arduino.h>
#include <MIDIUSB.h>

// Each button should be wired between its Arduino pin and GND.
// INPUT_PULLUP means:
//   HIGH = not pressed
//   LOW  = pressed

const byte midiChannel = 0;  // 0 = MIDI channel 1
const byte midiValue = 127;  // CC value sent when button is pressed, 0-127

const unsigned long debounceMs = 30;  // Button debounce time in milliseconds

struct MidiButton {
  byte pin;                         // Arduino input pin for this button
  byte cc;                          // MIDI CC number this button sends
  bool lastReading;                 // Last raw pin reading
  bool stableState;                 // Debounced button state
  unsigned long lastDebounceTime;   // Last time the raw reading changed
};

MidiButton buttons[] = {
  {2, 71, HIGH, HIGH, 0},  // Button on pin 2 sends CC 71
  {3, 72, HIGH, HIGH, 0},  // Button on pin 3 sends CC 72
  {4, 73, HIGH, HIGH, 0}   // Button on pin 4 sends CC 73
};

const byte buttonCount = sizeof(buttons) / sizeof(buttons[0]);

void sendMidiCC(byte channel, byte control, byte value) {
  midiEventPacket_t event = {
    0x0B,             // USB MIDI event type: Control Change
    0xB0 | channel,   // MIDI status byte: Control Change on selected channel
    control,          // MIDI CC number
    value             // MIDI CC value
  };

  MidiUSB.sendMIDI(event);
  MidiUSB.flush();

  Serial.print("Sent MIDI CC ");
  Serial.print(control);
  Serial.print(" value ");
  Serial.println(value);
}

void checkButton(MidiButton &button) {
  bool currentReading = digitalRead(button.pin);

  // If the input changed, restart the debounce timer.
  if (currentReading != button.lastReading) {
    button.lastDebounceTime = millis();
  }

  // If the input has stayed unchanged long enough, accept it as stable.
  if ((millis() - button.lastDebounceTime) > debounceMs) {
    if (currentReading != button.stableState) {
      button.stableState = currentReading;

      // With INPUT_PULLUP, LOW means the button is pressed.
      if (button.stableState == LOW) {
        sendMidiCC(midiChannel, button.cc, midiValue);
      }
    }
  }

  button.lastReading = currentReading;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  for (byte i = 0; i < buttonCount; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }

  Serial.println("Three-button MIDI CC test ready.");
}

void loop() {
  for (byte i = 0; i < buttonCount; i++) {
    checkButton(buttons[i]);
  }
}