#include <LazySerial.h>

// Either use the Serial Monitor (Tools->Serial Monitor), or you could try minicom.
// For Debian, you need to be in the 'dialout' group. e.g.:-
//     sudo useradd james dialout
// Then (after logging in again or starting a new login shell), you can
//     minicom --device /dev/ttyUSB0
// Use "Options->Screen->Add Carriage Return: Yes" as Minicom seems to render LF as plain LF.
// Also note there's no local echo enabled by default so you're typing blind. LazySerial is intended
// for a command-link between computer program and arduino, so a remote echo would be a bit pointless.

// check we're actually doing something, repeatedly blink led 13
int led_state = LOW;
int led_countdown_ms = 1000;
unsigned long last_time_ms = 0;

// Declare our global LazySerial object; tell it to use the Serial global object to communicate on.
LazySerial::LazySerial lazy(Serial);


// ######### CALLBACKS ########
// Define our callback functions first, or prototype them, so that register_callback("BLA", &func) doesn't error.

void say_hello(const char *blah)
{
  Serial.print("\n");
  Serial.println("Hello!");
  Serial.println("How are you?");
}

void say_good(const char *blah)
{
  Serial.print("\n");
  Serial.println("That's good!");  
}


// ######## MAIN ARDUINO FUNCTIONS ########

void setup()
{
  last_time_ms = millis();

  // register some commands for the serial interface.
  lazy.register_callback("HELLO", &say_hello);
  lazy.register_callback("GOOD",  &say_good);

  // We still need to do our own Serial init.
  Serial.begin(115200);
}

void loop()
{
  unsigned long current_time_ms = millis();
  
  // so we know it's doing something, keep a 'heartbeat' pulsing.
  led_countdown_ms -= current_time_ms - last_time_ms;
  if (led_countdown_ms < 0) {
    if (led_state == HIGH) {
      led_state = LOW;
      led_countdown_ms = 500;
    } else {
      led_state = HIGH;
      led_countdown_ms = 1000;
    }
    digitalWrite(13, led_state);   // set the internal LED
  }
  last_time_ms = current_time_ms;

  // Process commands via LazySerial!
  lazy.loop();
}



