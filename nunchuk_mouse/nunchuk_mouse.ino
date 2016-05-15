#include <Wire.h>
#include <ArduinoNunchuk.h>
#include <Mouse.h>

#define DEBUG
#define BAUDRATE 19200

#define X_MIN 20
#define X_CENTER 116
#define X_MAX 221

#define Y_MIN 26
#define Y_CENTER 124
#define Y_MAX 224

#define JITTER 2
#define OFFSET 3

#define MOUSE_SPEED 14
#define MOUSE_SCROLL_SPEED 18
#define C_ACTION MOUSE_LEFT
#define C_ACTION_2 MOUSE_RIGHT

enum direction {none, up, down, left, right};

enum direction prev_direction = none;
unsigned int i;

ArduinoNunchuk nunchuk = ArduinoNunchuk();

void setup() {
  Serial.begin(BAUDRATE);
  Mouse.begin();
  nunchuk.init();
}

void loop() {
  nunchuk.update();
  
  int xReading = nunchuk.analogX; // read x
  int yReading = nunchuk.analogY; // read y
  
  int x = 0;
  int y = 0;
  int scroll = 0;
  
  boolean z_pressed = nunchuk.zButton; // read z button
  boolean c_pressed = nunchuk.cButton; // read c button

  boolean movement = false;
  boolean scroll_override = true;
  
  if (xReading < X_CENTER - JITTER) { // x down
    if (!z_pressed) {
      x = map(xReading + OFFSET, X_MIN, X_CENTER, -MOUSE_SPEED, 0);
    } else {
      if (xReading < X_CENTER - 30) {
        if (prev_direction != left) {
          scroll = 1;
        }
        scroll_override = false;
        prev_direction = left;
      } else {
        //prev_direction = none;
      }
    }
    movement = true;
  } else if (xReading > X_CENTER + JITTER) { // x up
    if (!z_pressed) {
      x = map(xReading + OFFSET, X_CENTER, X_MAX, 0, MOUSE_SPEED);
    } else {
      if (xReading > X_CENTER + 30) {
        if (prev_direction != right) {
          scroll = -1;
        }
        scroll_override = false;
        prev_direction = right;
      } else {
        //prev_direction = none;
      }
    }
    movement = true;
  } else if (z_pressed) {
    prev_direction = none;
  }
  
  if (yReading < Y_CENTER - JITTER) { // y up
    if (!z_pressed) {
      y = map(yReading + OFFSET, Y_MIN, Y_CENTER, MOUSE_SPEED, 0);
    } else {
      int scroll_devider = map(yReading + OFFSET, Y_MIN, Y_CENTER, MOUSE_SCROLL_SPEED, 0);
      if (i % (100 / scroll_devider) == 0 && scroll_override) {
        scroll = -1;
      }
    }
    movement = true;
  } else if (yReading > Y_CENTER + JITTER) { // y down
    if (!z_pressed) {
      y = map(yReading + OFFSET, Y_CENTER, Y_MAX, 0, -MOUSE_SPEED);
    } else {
      int scroll_devider = map(yReading + OFFSET, Y_CENTER, Y_MAX, 0, MOUSE_SCROLL_SPEED);
      if (i % (100 / scroll_devider) == 0 && scroll_override) {
        scroll = 1;
      }
    }
    movement = true;
  }
  
  if (movement) {
    #ifdef DEBUG
      digitalWrite(13, HIGH);
    #endif
    Mouse.move(x, y, scroll);
  } else {
    #ifdef DEBUG
      digitalWrite(13, LOW);
    #endif
  }
  
  if (c_pressed) {
    if (z_pressed) {
     Mouse.release(C_ACTION);
      Mouse.press(C_ACTION_2);
    } else {
      Mouse.release(C_ACTION_2);
      Mouse.press(C_ACTION);
    }
  } else {
    Mouse.release(C_ACTION);
    Mouse.release(C_ACTION_2);
  }

  #ifdef DEBUG
    //Serial.print("x: ");
    Serial.print(nunchuk.analogX, DEC);
    Serial.print("\t");
    //Serial.print("y: ");
    Serial.print(nunchuk.analogY, DEC);/*
    Serial.print("\t");
    //Serial.print("ax: ");
    Serial.print(nunchuk.accelX, DEC);
    Serial.print("\t");
    Serial.print("ay: ");
    Serial.print(nunchuk.accelY, DEC);
    Serial.print("\t");
    Serial.print("az: ");
    Serial.print(nunchuk.accelZ, DEC);*/
    Serial.print("\t");
    //Serial.print("z: ");
    Serial.print(nunchuk.zButton, DEC);
    Serial.print("\t");
    //Serial.print("c: ");
    Serial.print(nunchuk.cButton, DEC);
    Serial.print("\t");
    //Serial.print("x: ");
    Serial.print(x, DEC);
    Serial.print("\t");
    //Serial.print("y: ");
    Serial.print(y, DEC);
    Serial.print("\t");
    //Serial.print("s: ");
    Serial.println(scroll, DEC);
  #endif

  i++;
  delay(2);
}
