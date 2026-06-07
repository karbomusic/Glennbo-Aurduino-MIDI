#include <Arduino.h>
#include <MIDIUSB.h>


// Button wiring:
// One side of the button goes to pin 2.
// The other side goes to GND.
// No external 10k resistor needed because we use INPUT_PULLUP.

const byte buttonPin = 2;

const byte midiChannel = 0;  // 0 = MIDI channel 1
const byte midiCC = 71;
const byte midiValue = 127;

const unsigned long debounceMs = 30;

bool lastReading = HIGH;
bool stableButtonState = HIGH;
unsigned long lastDebounceTime = 0;

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {
    0x0B,                 // USB MIDI event type: Control Change
    0xB0 | channel,        // MIDI status byte: CC on selected channel
    control,               // CC number
    value                  // CC value
  };

  MidiUSB.sendMIDI(event);
  MidiUSB.flush();
}

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  Serial.begin(115200);
  delay(1000);
}

void loop() {
  bool reading = digitalRead(buttonPin);

  // If the input changed, reset the debounce timer.
  if (reading != lastReading) {
    lastDebounceTime = millis();
  }

  // If the reading has stayed unchanged long enough, accept it.
  if ((millis() - lastDebounceTime) > debounceMs) {
    if (reading != stableButtonState) {
      stableButtonState = reading;

      // With INPUT_PULLUP, LOW means pressed.
      if (stableButtonState == LOW) {
        controlChange(midiChannel, midiCC, midiValue);

        // For debugging, print the MIDI message to the serial monitor.
          Serial.print("Sent MIDI CC ");
          Serial.print(midiCC);
          Serial.print(" value ");
          Serial.println(midiValue);
      }
    }
  }

  lastReading = reading;
  delay(10);  // Small delay to avoid busy looping
}