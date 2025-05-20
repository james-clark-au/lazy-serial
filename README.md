# LazySerial - Arduino Serial wrapper that exposes a command interface

When tinkering with Arduino, it's common to want to poke your code over the serial connection. LazySerial is my library to make it simple to add a bunch of text commands that will be dispatched to custom functions in your code.

## USAGE

To hook up the library to your code:

- Define an instance of the library, hooked up to your Serial object.
- Define some callback functions for each command you want to make. There is a LAZY_COMMAND(name) macro to use inside the function body to make it work.
- Define a static array of those commands, and call `.set_commands()` in your `setup()`.
- Call `.loop()` in your own `loop()` for command processing.

## EXAMPLE

```cpp
#include <LazySerial.h>

LazySerial::LazySerial<128> lazy(Serial);

// A command to identify yourself, essential when you have lots of projects hooked up over USB!
void cmd_ohai(LazySerial::Context &context) {
  LAZY_COMMAND("OHAI");
  context.stream.println(F("OHAI readme-example " __TIMESTAMP__  ));
}

// Commands can take arguments, and supply 'usage' text
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

// Quoted strings can be used for arguments, although these are indexed into LazySerial's buffer so copy them if you need 'em
void cmd_echo(LazySerial::Context &context) {
  LAZY_COMMAND("ECHO", "\"text string\"");
  char *text;
  bool ok = context.parse_string(&text);
  LAZY_RETURN_USAGE_UNLESS(ok);

  context.stream.printf("OK '%s'\n", text);
}

LazySerial::CallbackFunction commands[] = {
  cmd_ohai,
  cmd_gpio,
  cmd_say,
};

void setup() {
  Serial.begin(9600);  // Don't forget to actually initialise Serial yourself!
  lazy.set_commands(commands);
}

void loop() {
  lazy.loop();
}
```

## LazySerial object

### LazySerial(Stream &stream)

The constructor just takes a Stream object, most likely your Serial global but you could swap in different Serial ports if your platform provides them, or set up something fun like taking commands over wifi.

The instance takes a template argument - this specifies how big the internal char* buffer for the commands is. For severely memory-limited platforms, you may want to reduce this.

```cpp
LazySerial::LazySerial<128> lazy(Serial);
```

### void set_commands(CallbackFunction *commands)

To be called in `setup()`, this will associate your statically-declared array of command callbacks with the LazySerial instance. Magic voodoo template shenanigans make the function deduce the array size automagically, presuming you are passing in an actual array.

```cpp
LazySerial::CallbackFunction commands[] = {
  cmd_ohai,
  cmd_gpio,
  cmd_say,
};
... later ...
lazy.set_commands(commands);
```

### void loop()

Call this from within your own `loop()`. It checks the Serial for more characters, and if it manages to build a string in its buffer that is terminated with a CR or LF terminator, it dispatches that to one of your defined commands.

If the buffer overflows, the input is discarded.

If no command matches, a built-in 'HELP' command is run. This lists out all the registered commands by name. You can swap out a different implementation using `.set_help_callback(CallbackFunction &)`

### void run_script(const char *script)

Run one or more '\n'-delimited commands in sequence. The final command does not need a '\n'.

### void run_script(ReaderFunction read_char_fn)

If you have a script stored in e.g. EEPROM, you might not want to load the whole thing into memory just to load _sections_ of it into the LazySerial command buffer and then execute them. This is a variation of `run_script(const char *)` that instead lets the user supply a function to do the reading: It should be a function whose signature is `char fun(size_t pos)`.

### void dispatch_command(const char *cmd_name, char *cmd_args)

Dispatch a command directly by name. You probably want to use `run_script()` instead; note the string for the `cmd_args` may have a few '\0' characters jammed into it to aid parsing.

### void cmd_help()

Trigger the builtin help command.

## Context object

The callback functions you define for your commands all follow the same pattern:

```cpp
void mycommand(LazySerial::Context &context);
```

The context object passed in controls how the command is being invoked. If you're using the `LAZY_COMMAND()` macro you won't have to think about it, but the function can be called for multiple reasons:

- To print the name of the function (as used by the HELP command)
- To print the usage text of the command, if there's a parsing problem
- To actually perform the work associated with the command


### bool parse_int(T *var, bool expect_hex = false)

You can use `context.args` to access the args buffer directly, but a few convenience methods are also available on the Context object. `.parse_int(&myInt)` takes an integer variable by address, and attempts to parse one out of the args buffer. It will return false on failure, not modifying the passed variable.

It will automatically detect an '0x' prefix indicating a hex value, but if you're expecting a hex number without the prefix, set `expect_hex` to true.

### bool parse_int_minmax(T *var, T min, T max, bool expect_hex = false)

As with `parse_int()`, but will return `false` if the supplied number falls outside of the (inclusive) min,max range.

Note that when calling this one, you may need to explicitly supply the templated int type, i.e.

```cpp
bool ok = context.parse_int_minmax<uint8_t>(&pinNum, 0, 255);
```
### bool parse_float(T *var)

As with `parse_int()` but for floats or doubles.

### bool parse_float_minmax(T *var, T min, T max)

As with `parse_float()`, but again specifying a min and max acceptable range.

### bool parse_word(char **charstar_ptr)

This one will look for the next space-delimited word in the args buffer, and set the supplied char* to point to it. It modifies the args buffer to do this, changing the first character past the word to '\0'.

```cpp
char *onoff;
bool ok = context.parse_word(&onoff);
```

### bool parse_string(char **charstar_ptr, bool bareword_ok = false)

Parses a double-quoted string out of the args buffer. If you set `bareword_ok`, and the parser encounters something other than a '"', the behaviour will fall back to `parse_word()`.

Escape sequences are not processed, although `\"` will be skipped over in the hunt for the terminating '"'.

## HELPER MACROS

### LAZY_COMMAND(name, usage)

This is the macro provided to handle the 'context' parameter passed to your command. It assumes this parameter is called `context` and will break if it isn't. The 'name' parameter is a string literal to use for your command's name, and will be case-insensitively compared to the input buffer.

The 'usage' argument is optional, but recommended. If your command sets `context.mode = LazySerial::CallingMode::USAGE` and returns, it will be re-called with this set, so that the macro knows to print out the usage message.

If using the macro - which is highly recommended - you can assume that if execution progresses past it, your command is being invoked to do its assigned work.

```cpp
void cmd_gpio(LazySerial::Context &context) {
  LAZY_COMMAND("GPIO", "<pin number> <ON|OFF>");
```

### LAZY_RETURN_USAGE_IF(cond) LAZY_RETURN_USAGE_UNLESS(cond)

These two helper macros set `context.mode = LazySerial::CallingMode::USAGE` and immediately return.

The way LazySerial dispatches commands, this will trigger it to re-run your callback with that mode still set, which the `LAZY_COMMAND()` macro will then understand as a request to print the usage string.

### LAZY_KEYVAL(defined_var_name)

Stringifies a " varname=varvalue", which I typically use in a "PINOUT" command to remind me what's hooked up to what.

```cpp
  context.stream.println(F("OK PINOUT" LAZY_KEYVAL(PIN_LED) LAZY_KEYVAL(PIN_CLK) LAZY_KEYVAL(PIN_DIO) LAZY_KEYVAL(PIN_SENSOR) ));
```

## LICENCE

MIT.
