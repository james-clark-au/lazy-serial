#pragma once

namespace BlinkyLed
{
  static const uint8_t NO_PIN = 255;

  class BlinkyLed
  {
  public:
    uint8_t  d_pin      = NO_PIN;
    uint8_t  d_state    = LOW;
    uint32_t d_interval = 0;
  
    BlinkyLed() {  }

    BlinkyLed(uint8_t pin, uint32_t interval_ms = 0) {
      d_pin = pin;
      d_interval = interval_ms;
      pinMode(pin, OUTPUT);
    }

    explicit
    operator bool() const {
      return d_pin != NO_PIN;
    }
  
    uint8_t
    pin() {
      return d_pin;
    }

    void
    changePin(uint8_t pin) {
      // Bring old pin low first
      set(LOW);
      // Set new pin
      d_pin = pin;
      pinMode(d_pin, OUTPUT);
      digitalWrite(d_pin, d_state);
    }
  
    /*
     * Update interval between on/off toggles.
     * Set to 0 to just be on/off all the time.
     */
    void
    changeInterval(uint32_t interval_ms) {
      set(LOW);
      d_interval = interval_ms;
    }
  
    void
    set(uint8_t state) {
      if (d_pin == NO_PIN) {
        return;
      }
      d_state = state;
      digitalWrite(d_pin, d_state);
    }

    void
    off() {
      changeInterval(0);
      set(LOW);
    }

    void
    on() {
      changeInterval(0);
      set(HIGH);
    }

    void
    blink(uint32_t interval_ms) {
      set(HIGH);
      d_interval = interval_ms;
    }

    void
    loop() {
      if (d_interval == 0 || d_pin == NO_PIN) {
        return;
      }
      uint32_t heartBeat = millis() % (d_interval * 2);
      if (heartBeat < d_interval && d_state != LOW) {
        set(LOW);
      } else if (heartBeat >= d_interval && d_state != HIGH) {
        set(HIGH);
      }
    }
  };

} // namespace

