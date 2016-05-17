#include <Wire.h>
#include <ArduinoNunchuk.h>
#include <Keyboard.h>
#include <Mouse.h>

//#define DEBUG
#define BAUDRATE 19200

#define X_MIN     20
#define X_CENTER 116
#define X_MAX    221

#define Y_MIN     26
#define Y_CENTER 124
#define Y_MAX    224

#define JITTER 2
#define OFFSET 3

#define MOUSE_SPEED         14
#define MOUSE_SCROLL_SPEED  18
#define C_ACTION    MOUSE_LEFT
#define C_ACTION_2 MOUSE_RIGHT
#define Z_TOGGLE_TIME     500

#define ACCEL_LEFT     400
#define ACCEL_RIGHT    600
#define ACCEL_FORWARD  580
#define ACCEL_BACKWARD 440
#define ACCEL_SHAKE    800

// use WASD and Space to move
#define ACCEL_LEFT_KEY     'a'
#define ACCEL_RIGHT_KEY    'd'
#define ACCEL_FORWARD_KEY  'w'
#define ACCEL_BACKWARD_KEY 's'
#define ACCEL_SHAKE_KEY    ' '

enum direction {none, up, down, left, right, forward, backward};
enum direction prev_joystick_direction = none;
enum direction prev_accelerometer_direction = none;

unsigned int i = 0;
unsigned long z_pressed_prev_time = 0;
unsigned int z_pressed_counter = 0;
boolean z_pressed_prev = false;
boolean use_accelerometer = false;

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
        if (prev_joystick_direction != left) {
          scroll = 1;
        }
        scroll_override = false;
        prev_joystick_direction = left;
      } else {
        //prev_joystick_direction = none;
      }
    }
    movement = true;
  } else if (xReading > X_CENTER + JITTER) { // x up
    if (!z_pressed) {
      x = map(xReading + OFFSET, X_CENTER, X_MAX, 0, MOUSE_SPEED);
    } else {
      if (xReading > X_CENTER + 30) {
        if (prev_joystick_direction != right) {
          scroll = -1;
        }
        scroll_override = false;
        prev_joystick_direction = right;
      } else {
        //prev_joystick_direction = none;
      }
    }
    movement = true;
  } else if (z_pressed) {
    prev_joystick_direction = none;
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

  if (z_pressed_prev && !z_pressed) {
    if (millis() - z_pressed_prev_time > Z_TOGGLE_TIME) { // check if button press took too long
      Serial.println(String("button press took too long (") + (millis() - z_pressed_prev_time) + "ms)");
      z_pressed_counter = 0;  // reset counter
    }
    
    z_pressed_counter ++; // increment counter
    z_pressed_prev_time = millis();
    
    Serial.println(String("pressed ") + z_pressed_counter + " times");
    
    if (z_pressed_counter == 3) {
      use_accelerometer = !use_accelerometer; // toggle accelerometer mode
      z_pressed_counter = 0;  // reset counter

      digitalWrite(13, use_accelerometer);

      if (use_accelerometer) {
        Keyboard.begin();
      }
      
      Serial.println(String("accelerometer mode: ") + (use_accelerometer ? "on" : "off"));
    }
  }

  if (use_accelerometer) {
    if (nunchuk.accelX < ACCEL_LEFT) {
      Keyboard.press(ACCEL_LEFT_KEY);
      Keyboard.release(ACCEL_RIGHT_KEY);
    } else if (nunchuk.accelX > ACCEL_RIGHT) {
      Keyboard.press(ACCEL_RIGHT_KEY);
      Keyboard.release(ACCEL_LEFT_KEY);
    } else {
      Keyboard.release(ACCEL_RIGHT_KEY);
      Keyboard.release(ACCEL_LEFT_KEY);
    }
    
    if (nunchuk.accelY < ACCEL_BACKWARD) {
      Keyboard.press(ACCEL_BACKWARD_KEY);
      Keyboard.release(ACCEL_FORWARD_KEY);
    } else if (nunchuk.accelY > ACCEL_FORWARD) {
      Keyboard.press(ACCEL_FORWARD_KEY);
      Keyboard.release(ACCEL_BACKWARD_KEY);
    } else {
      Keyboard.release(ACCEL_BACKWARD_KEY);
      Keyboard.release(ACCEL_FORWARD_KEY);
    }

    if (nunchuk.accelZ > ACCEL_SHAKE) {
      Keyboard.press(ACCEL_SHAKE_KEY);
    } else {
      Keyboard.release(ACCEL_SHAKE_KEY);
    }
  }
  
  if (movement) {
    Mouse.move(x, y, scroll);
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

  //Serial.print("Orientation: ");
  /*Serial.print(nunchuk.accelX, DEC);
  Serial.print(" ");
  Serial.print(nunchuk.accelY, DEC);
  Serial.print(" ");
  Serial.print(nunchuk.accelZ, DEC);
  Serial.println("");*/

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

  z_pressed_prev = z_pressed;
  i++;
  delay(2);
}
