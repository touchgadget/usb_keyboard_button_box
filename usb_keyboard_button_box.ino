/*
MIT License

Copyright (c) 2021 touchgadgetdev@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
/* USB Keyboard Button Box with 11 buttons and 3 rotary encoders.
 *
 * Tested on Playstation 4 with F1 2020 racing game. Note a second
 * keyboard can be plugged in at the same time. For example, a number
 * keypad can be used for buttons and this project for rotary encoders.
 */

#include "Adafruit_seesaw.h"  // Install using IDE library manager
#include <seesaw_neopixel.h>
#include <Bounce2.h>          // Install using IDE library manager
#include <Keyboard.h>

/* Make this true only for debugging because it slows down sketch. */
#define DEBUG_ON  false

#if DEBUG_ON
#define DEBUG_begin(...)  Serial.begin(__VA_ARGS__)
#define DEBUG_print(...)  Serial.print(__VA_ARGS__)
#define DEBUG_println(...)  Serial.println(__VA_ARGS__)
#else
#define DEBUG_begin(...)
#define DEBUG_print(...)
#define DEBUG_println(...)
#endif

/* Use GPIO pins as button inputs */
#define NUM_BUTTONS 11
Bounce * buttons = new Bounce[NUM_BUTTONS];

typedef struct {
  const uint8_t pin_number;
  const uint8_t usb_keycode;
  bool key_down;
} usb_key_pins_t;

usb_key_pins_t USB_Keys[NUM_BUTTONS] = {
  {A0, 'a', false},
  {A1, 'b', false},
  {A2, 'c', false},
  {A3, 'd', false},
  {SDA, 'e', false},
  {SCL, 'f', false},
  {PIN_SERIAL1_TX, 'g', false},
  {PIN_SERIAL1_RX, 'h', false},
  {SCK, 'i', false},
  {MISO, 'j', false},
  {MOSI, 'k', false},
};

/* Setup for I2C rotary encoder boards */
#define SS_SWITCH         24      // this is the pin on the encoder connected to switch
#define SS_NEOPIX          6      // this is the pin on the encoder connected to neopixel

#define SEESAW_BASE_ADDR  0x36  // I2C address, starts with 0x36
#define NUM_ENCODERS       3

// create encoders!
Adafruit_seesaw encoders[NUM_ENCODERS];
// create encoder pixels
seesaw_NeoPixel encoder_pixels[NUM_ENCODERS] = {
  seesaw_NeoPixel(1, SS_NEOPIX, NEO_GRB + NEO_KHZ800),
  seesaw_NeoPixel(1, SS_NEOPIX, NEO_GRB + NEO_KHZ800),
  seesaw_NeoPixel(1, SS_NEOPIX, NEO_GRB + NEO_KHZ800)};

int32_t encoder_positions[NUM_ENCODERS] = {0, 0, 0};
bool found_encoders[NUM_ENCODERS] = {false, false, false};
bool encoder_buttons[NUM_ENCODERS] = {false, false, false};

const uint8_t USB_KEYS_DOWN[NUM_ENCODERS] = {KEY_F1, KEY_F3, KEY_F5};
const uint8_t USB_KEYS_UP[NUM_ENCODERS] = {KEY_F2, KEY_F4, KEY_F6};
const uint8_t USB_KEYS_BUTTON[NUM_ENCODERS] = {KEY_F7, KEY_F8, KEY_F9};

void setup()
{
#if DEBUG_ON
  DEBUG_begin(115200);

  // wait for serial port to open
  while (!Serial) delay(10);
#endif

  DEBUG_println("USB Keyboard Button Box");

  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].attach( USB_Keys[i].pin_number , INPUT_PULLUP  );
    buttons[i].interval(5);
  }

  DEBUG_println("Looking for rotary encoder seesaws!");
  for (uint8_t enc=0; enc < NUM_ENCODERS; enc++) {
    // See if we can find encoders on this address
    if (! encoders[enc].begin(SEESAW_BASE_ADDR + enc) ||
        ! encoder_pixels[enc].begin(SEESAW_BASE_ADDR + enc)) {
      DEBUG_print("Couldn't find encoder #");
      DEBUG_println(enc);
    }
    else {
      DEBUG_print("Found encoder + pixel #");
      DEBUG_println(enc);

      uint32_t version = ((encoders[enc].getVersion() >> 16) & 0xFFFF);
      if (version != 4991) {
        DEBUG_print("Wrong firmware loaded? ");
        DEBUG_println(version);
        while(1) delay(10);
      }
      DEBUG_println("Found Product 4991");

      // use a pin for the built in encoder switch
      encoders[enc].pinMode(SS_SWITCH, INPUT_PULLUP);

      // get starting position
      encoder_positions[enc] = encoders[enc].getEncoderPosition();

      DEBUG_println("Turning on interrupts");
      delay(10);
      encoders[enc].setGPIOInterrupts((uint32_t)1 << SS_SWITCH, 1);
      encoders[enc].enableEncoderInterrupt();

      // set not so bright!
      encoder_pixels[enc].setBrightness(30);
      encoder_pixels[enc].show();

      found_encoders[enc] = true;
    }
  }

  DEBUG_println("Encoders started");
  Keyboard.begin();
}

void loop()
{
  for (int pin = 0; pin < NUM_BUTTONS; pin++) {
    buttons[pin].update();
    if (buttons[pin].fell()) {
      if (!USB_Keys[pin].key_down) {
        DEBUG_print("pin_number ");
        DEBUG_print(USB_Keys[pin].pin_number);
        DEBUG_print(" press usb_keycode ");
        DEBUG_println(USB_Keys[pin].usb_keycode, HEX);
        Keyboard.press(USB_Keys[pin].usb_keycode);
        USB_Keys[pin].key_down = true;
      }
    }
    else if (buttons[pin].rose()) {
      if (USB_Keys[pin].key_down) {
        DEBUG_print("pin_number ");
        DEBUG_print(USB_Keys[pin].pin_number);
        DEBUG_print(" release usb_keycode ");
        DEBUG_println(USB_Keys[pin].usb_keycode, HEX);
        Keyboard.release(USB_Keys[pin].usb_keycode);
        USB_Keys[pin].key_down = false;
      }
    }
  }

  for (int enc = 0; enc < NUM_ENCODERS; enc++) {
    if (found_encoders[enc] == false) continue;

    // Handle encoder button presses and releases
    int new_button = encoders[enc].digitalRead(SS_SWITCH);
    if (encoder_buttons[enc] != new_button) {
      if (new_button) {
        Keyboard.release(USB_KEYS_BUTTON[enc]);
        encoder_buttons[enc] = false;
        DEBUG_print("Encoder button #");
        DEBUG_print(enc);
        DEBUG_println(" released");
      }
      else {
        Keyboard.press(USB_KEYS_BUTTON[enc]);
        encoder_buttons[enc] = true;
        DEBUG_print("Encoder button #");
        DEBUG_print(enc);
        DEBUG_println(" pressed");
      }
      encoder_buttons[enc] = new_button;
    }

    // Handle encoder rotation
    int32_t new_position = encoders[enc].getEncoderPosition();
    if (encoder_positions[enc] != new_position) {
      DEBUG_print("Encoder #");
      DEBUG_print(enc);
      DEBUG_print(" -> ");
      DEBUG_println(new_position);

      // Clock wise rotation
      if (new_position < encoder_positions[enc]) {
        Keyboard.write(USB_KEYS_UP[enc]);
        encoder_pixels[enc].setPixelColor(0, 0xFF0000);
        encoder_pixels[enc].show();
      }
      // Counter clock wise rotation
      else if (new_position > encoder_positions[enc]) {
        Keyboard.write(USB_KEYS_DOWN[enc]);
        encoder_pixels[enc].setPixelColor(0, 0x00FF00);
        encoder_pixels[enc].show();
      }
      encoder_positions[enc] = new_position;
    }
    else {
      encoder_pixels[enc].setPixelColor(0, 0);
      encoder_pixels[enc].show();
    }
  }

#if DEBUG_ON
  delay(10);
#endif
}
