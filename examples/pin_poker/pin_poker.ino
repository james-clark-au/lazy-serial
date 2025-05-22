/*
 * pin-poker - a bunch of serial commands to let you interactively turn digital pins on/off, or read values.
 *
 * This file is part of the LazySerial library example code. It is licenced under the MIT Open Source licence.
 * See the file LICENCE for details.
 * Copyright (C) 2025 James Neko <arduino@neko.stream>
 * 
 * SPDX-License-Identifier: MIT
 */ 
#include "BlinkyLed.h"
#include "Ticker.h"
#include <LazySerial.h>

#define BAUD_RATE 9600

static_assert(LAZYSERIAL_VERSION >= 2.0);
LazySerial::LazySerial<128> lazy(Serial);
BlinkyLed::BlinkyLed blinky(LED_BUILTIN, 1000);
int monitorPin = -1;
bool monitorDigital = true;
int monitorFps = 10;
int val;
int beepPin = -1;
int beepMs = 1000;

// ---------------- Digital / Analogue read  ----------------
void ticker_read_val() {
  LAZY_RETURN_IF(monitorPin == -1);
  Serial.print("MONITOR PIN ");
  Serial.print(monitorPin);
  Serial.print(" ");
  if (monitorDigital) {
    val = digitalRead(monitorPin);
    if (val == HIGH) {
      Serial.println("HIGH");
    } else {
      Serial.println("LOW");
    }
  } else {
    val = analogRead(monitorPin);
    Serial.print(" VALUE ");
    Serial.println(val);
  }
}
Ticker::Ticker ticker(monitorFps, ticker_read_val);


// ---------------- SERIAL COMMAND CALLBACKS ----------------

void cmd_ohai(LazySerial::Context &context) {
  LAZY_COMMAND("OHAI");
  context.stream.println(F("OHAI pin_poker " __TIMESTAMP__  ));
}

void cmd_pinout(LazySerial::Context &context) {
  LAZY_COMMAND("PINOUT");
  context.stream.println(F("OK PINOUT" LAZY_KEYVAL(ARDUINO_BOARD) LAZY_KEYVAL(LED_BUILTIN) ));
}

void cmd_blink(LazySerial::Context &context) {
  LAZY_COMMAND("BLINK", "<pinNum>");
  uint8_t pinNum = 0;
  bool ok = context.parse_int_minmax<uint8_t>(&pinNum, 0, 255);
  LAZY_RETURN_USAGE_UNLESS(ok);

  blinky.setPin(pinNum);
  context.stream.print("OK BLINK ");
  context.stream.println(pinNum);
}

void cmd_pinmode(LazySerial::Context &context) {
  LAZY_COMMAND("PINMODE", "<pinNum> (INPUT|INPUT_PULLUP|INPUT_PULLDOWN|OUTPUT)");
  uint8_t pinNum = 0;
  char *mode;
  bool ok = context.parse_int_minmax<uint8_t>(&pinNum, 0, 255);
  LAZY_RETURN_USAGE_UNLESS(ok);
  ok = context.parse_word(&mode);
  LAZY_RETURN_USAGE_UNLESS(ok);

  if (strcasecmp(mode, "INPUT") == 0) {
    pinMode(pinNum, INPUT);
  } else if (strcasecmp(mode, "INPUT_PULLUP") == 0) {
    pinMode(pinNum, INPUT_PULLUP);
  } else if (strcasecmp(mode, "INPUT_PULLDOWN") == 0) {
    pinMode(pinNum, INPUT_PULLDOWN);
  } else if (strcasecmp(mode, "OUTPUT") == 0) {
    pinMode(pinNum, OUTPUT);
  } else {
    LAZY_RETURN_USAGE_IF(true);
  }
  context.stream.print("OK PINMODE ");
  context.stream.print(pinNum);
  context.stream.print(" ");
  context.stream.println(mode);
}

void cmd_gpio(LazySerial::Context &context) {
  LAZY_COMMAND("GPIO", "<pin number> <ON|OFF>");
  uint8_t pin = 0;
  char *onoff;
  bool ok = context.parse_int(&pin);
  LAZY_RETURN_USAGE_UNLESS(ok);
  ok = context.parse_word(&onoff);
  LAZY_RETURN_USAGE_UNLESS(ok);

  pinMode(pin, OUTPUT);
  context.stream.print("OK GPIO ");
  context.stream.print(pin);
  if (strcasecmp(onoff, "ON") == 0) {
    digitalWrite(pin, HIGH);
    context.stream.print(" ON\n");
  } else {
    digitalWrite(pin, LOW);
    context.stream.print(" OFF\n");
  }
}

void cmd_monitor(LazySerial::Context &context) {
  LAZY_COMMAND("MONITOR", "(PIN <pinNum>|FPS <fps>|OFF|ANALOG|DIGITAL)+");
  LAZY_RETURN_USAGE_UNLESS(*(context.pos));

  char *word;
  while (context.parse_word(&word)) {
    if (strcasecmp(word, "PIN") == 0) {
      bool ok = context.parse_int(&monitorPin);
      LAZY_RETURN_USAGE_UNLESS(ok);
      
    } else if (strcasecmp(word, "FPS") == 0) {
      bool ok = context.parse_int(&monitorFps);
      LAZY_RETURN_USAGE_UNLESS(ok);
      ticker.tps(monitorFps);
      
    } else if (strcasecmp(word, "OFF") == 0) {
      monitorPin = -1;

    } else if (word[0] == 'a' || word[0] == 'A') {
      monitorDigital = false;

    } else if (word[0] == 'd' || word[0] == 'D') {
      monitorDigital = true;
    }
  }

  context.stream.print("OK MONITOR");
  if (monitorPin == -1) {
    context.stream.println(" OFF");
    return;
  }
  context.stream.print(" PIN ");
  context.stream.print(monitorPin);
  context.stream.print(" FPS ");
  context.stream.print(monitorFps);
  if (monitorDigital) {
    context.stream.println(" DIGITAL");
  } else {
    context.stream.println(" ANALOG");
  }
}

void cmd_beep(LazySerial::Context &context) {
  LAZY_COMMAND("BEEP", "(PIN <pinNum>|TIME <ms>|STOP|<frequency>)+");
  LAZY_RETURN_USAGE_UNLESS(*(context.pos));

  char *word;
  while (context.parse_word(&word)) {
    if (strcasecmp(word, "PIN") == 0) {
      bool ok = context.parse_int(&beepPin);
      LAZY_RETURN_USAGE_UNLESS(ok);
      
    } else if (strcasecmp(word, "TIME") == 0) {
      bool ok = context.parse_int(&beepMs);
      LAZY_RETURN_USAGE_UNLESS(ok);
      
    } else if (strcasecmp(word, "STOP") == 0) {
      context.stream.print("OK BEEP STOP");
      noTone(beepPin);

    } else {
      int freq = atoi(word);
      context.stream.print("OK BEEP ");
      context.stream.println(freq);
      tone(beepPin, freq, beepMs);
      return;
    }
  }

  context.stream.print("OK BEEP");
  context.stream.print(" PIN ");
  context.stream.print(beepPin);
  context.stream.print(" TIME ");
  context.stream.println(beepMs);
}

LazySerial::CallbackFunction commands[] = {
  cmd_ohai,
  cmd_pinout,
  cmd_blink,
  cmd_pinmode,
  cmd_gpio,
  cmd_monitor,
  cmd_beep,
};


void setup() {
  Serial.begin(BAUD_RATE);
  lazy.set_commands(commands);
  Serial.println("OK STARTING");
}

void loop() {
  lazy.loop();
  blinky.loop();
  ticker.loop();
}

