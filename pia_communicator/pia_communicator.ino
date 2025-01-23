#include <Arduino.h>
#include <MCP23S17.h>
#include <SPI.h>

#define DEBUG 0  // Debug level, retained from original code,
                 // but not actively updated

#define IO_SS 10      // Extender /CS (Arduino pin)

#define IO_VIDEO 0    // Extender port for video data
#define IO_VIDEO_D0 0 // (extender pin)
#define IO_VIDEO_D6 6 // (extender pin)
#define IO_VIDEO_D7 7 // Not connected (extender pin)

#define VIDEO_DA  3   // Video data ready (active high) (Arduino pin)
#define VIDEO_RDA 5   // Video data ack (active high) (Arduino pin)

#define CLOCK_PIN 6   // Connected, not used (Arduino pin)
#define RESET_PIN 7   // SBC RESET signal (Arduino pin)

#define IO_KBD 1      // Extender port for kbd data
#define IO_KBD_D0 8   // (extender pin)
#define IO_KBD_D7 15  // (extender pin)

#define KBD_READY  2  // CPU ready for keyboard data (active low) (Arduino pin)
#define KBD_STROBE 4  // Keyboard data strobe (active high) (Arduino pin)

#define KBD_TOUT_MS 200 // Wait this long for CPU to grab keyboard data


MCP23S17 bridge(&SPI, IO_SS, 0);


void bridge_init() {
  bridge.begin();

  // Configure video inputs
  for (int i = IO_VIDEO_D0; i <= IO_VIDEO_D6; i++)
    bridge.pinMode(i, INPUT);
  bridge.pinMode(IO_VIDEO_D7, INPUT_PULLUP); // Not connected

  // Configure keyboard outputs
  for (int i = IO_KBD_D0; i <= IO_KBD_D7; i++)
    bridge.pinMode(i, OUTPUT);
}





void print_hex(byte value, bool newline = true) {
  Serial.print(value <= 0xF ? "0x0" : "0x");
  Serial.print(value, HEX);
  if (newline) Serial.println();
}

void debug_value(String description, byte value, int level = 1) {
  if (level > DEBUG) return;
  Serial.print(description);
  Serial.print(": ");
  print_hex(value);
}

void output_status() {
  debug_value("Video DA", digitalRead(VIDEO_DA));
  debug_value("Video D0-D6", bridge.readPort(IO_VIDEO) & 127);
  debug_value("Keyboard RDY", digitalRead(KBD_READY));
}








char map_to_ascii(int c) {
  /* Convert ESC key */
  //if (c == 0xcb) c = 27; // ???

  /* Ctrl A-Z */
  //if (c > 576 && c < 603) c -= 576; // ???

  // lowercase key to uppercase
  if (c >= 'a' && c <= 'z')
    c = c - 'a' + 'A';
  
  return c;
}




// !=0: keyboard timeout active, value corresponds to
//      millis() of last key data send
// ==0: keyboard timeout passed
int last_kbd_ms = 0;


void send_kbd_data(int c) {
  // Write kbd data  
  bridge.writePort(IO_KBD, c | 0x80);

  // == Strobe keyboard data
  // Original hw doesn't use CA2 PIA line (KBD_READY), but
  // using it here can help with "modern" connectivity like
  // pasting characters in serial terminal.
  // HW note: While not crucial measurements suggest at least
  //     4k7 pullup on CA2 line for better response and noise
  //     immunity.
  // We need to act in microsecond range to catch READY changes.
  // Fastest CPU reassert of keyboard ready line is about 6us
  // (with 1MHz CPU clock)
  noInterrupts();
  PORTD |= 1<<KBD_STROBE; //digitalWrite(KBD_STROBE, HIGH);
  for(int z = 0; z<10; z++)
    if ((PIND & (1<<KBD_READY))) break; //digitalRead(KBD_READY) == HIGH
  
  PORTD &=~(1<<KBD_STROBE); //digitalWrite(KBD_STROBE, LOW);
  interrupts();

  last_kbd_ms = millis()|1;
}


void handle_kbd_in() {

  int kbd_ready = digitalRead(KBD_READY) == LOW;

  // Process keyboard timeout
  int ms = millis();
  if (last_kbd_ms && (ms-last_kbd_ms) > KBD_TOUT_MS)
    last_kbd_ms = 0; // Deactivate timeout

  if (last_kbd_ms && !kbd_ready)
    return; // no timeout and CPU not ready for kbd data

  if (Serial.available() <= 0)
    return; // No data available

  // Read serial data
  int c = Serial.read();
  c = map_to_ascii(c);
  if (c<0 || c >= 0x60)
    return; // Invalid data

  send_kbd_data(c);
}





void serial_send_char(char c) {
  if (DEBUG >= 5) Serial.print("[");
  // Check valid range for output characters
  // - based on expected output of TEST PROGRAM
  //   from "Apple-1 Operation Manual" (pg.2)
  if (c == '\r' || (c >= ' ' && c < 0x7f)) {
    if (c == '\r') c = '\n'; // Replace CR with LF
    Serial.print(c);
  }
  if (DEBUG >= 5) Serial.print("]");
}


void handle_video_out() {
  if (digitalRead(VIDEO_DA) == HIGH) {
    char c = bridge.readPort(IO_VIDEO) & 0x7f;
    debug_value("VID", c, 10);
    serial_send_char(c);

    // == Acknowledge video data
    // We want to raise ack signal (VIDEO_RDA), wait until
    // video data ready (VIDEO_DA) goes down. Then deactivate
    // ack signal before CPU manages to make ready further
    // video data (raising VIDEO_DA again). We have to react
    // in microseconds range. Fastest PIA response time measured
    // was around 150ns and fastest CPU reassert of video output
    // ready is about 7us (with 1MHz clock).
    // digitalRead/digitalWrite isn't fast enough to always catch
    // the drop on the video data ready line. Thus, using direct
    // register access.
    noInterrupts();
    PORTD |= 1<<VIDEO_RDA; //digitalWrite(VIDEO_RDA, HIGH);
    for(int z = 0; z<10; z++)
      if ((PIND & (1<<VIDEO_DA)) == 0) break;
    
    PORTD &=~(1<<VIDEO_RDA); //digitalWrite(VIDEO_RDA, LOW);
    interrupts();
  }
}


void after_reset_init() {
  last_kbd_ms = 0;
}


void handle_reset() {
  while (digitalRead(RESET_PIN) == LOW) {

    after_reset_init();

    // Wait until RESET line is released
    // .. with some debouncing
    for(int z=0; z<10000; z++)
      if (digitalRead(RESET_PIN) == LOW) z=0;

  }
}


void setup() {
  Serial.begin(115200);

  pinMode(RESET_PIN, INPUT);

  pinMode(KBD_READY, INPUT);
  pinMode(VIDEO_DA, INPUT);
  pinMode(KBD_STROBE, OUTPUT);
  pinMode(VIDEO_RDA, OUTPUT);

  bridge_init();
  digitalWrite(VIDEO_RDA, LOW);  // Video ack not active
  digitalWrite(KBD_STROBE, LOW); // Kbd strobe inactive
  
  after_reset_init();

  Serial.println("\nRC6502 Apple 1 Replica");
  Serial.println("(PIA Communicator NG)");

  output_status();
}


void loop() {
  handle_reset();
  handle_kbd_in();
  handle_video_out();
}


