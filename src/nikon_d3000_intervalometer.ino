/**
 * nikon_d3000_intervalometer.ino
 * A simple budget intervalometer for the Nikon D3000 camera
 * built for the Arduino Nano board.
 * Copyright (c) by Sam Kitano 21.08.2020
 * This work is licensed under a MIT style license. See LICENSE for more info.
 */

#include <SPI.h>
#include <Wire.h>
#include "OneButton.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED DISPLAY (SSD1306) DEFS
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// PINOUT
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define UP_BTN_PIN  2 // Up button
#define DN_BTN_PIN  3 // Down button
#define SEL_BTN_PIN 4 // Selection button
#define IR_PIN      5 // IR LED
#define BUZZ_PIN    6 // Buzzer

// GLOBAL VARS
boolean buttonActive    = false;
boolean longPressActive = false;

unsigned long buttonTimer    = 0;
unsigned long longPressTime  = 500;
unsigned long shortPressTime = 10;

unsigned int shots    =  5; // Shots to take. Minimum is 1
unsigned int interval =  1; // Interval between shots
unsigned int wait     = 10; // Default delay before start taking shots (seconds)
unsigned int dCycle   =  0; // IR Duty cycle
unsigned int page     =  1; // Menu page

// GLOBAL CONSTANTS
unsigned const int steps       =    5; // Steps to fast increase/decrease values
unsigned const int maxWait     =   60; // Max allowed time to delay photoshoot (seconds)
unsigned const int maxShots    =  999; // Max allowed shots
unsigned const int maxInterval = 3600; // Max delay between shots
unsigned const int minWait     =    0; // Min allowed time to delay photoshoot (seconds)
unsigned const int minShots    =    1; // Min allowed shots
unsigned const int minInterval =    1; // Min allowed delay between shots

// CLASS INSTANTIATIONS
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
OneButton buttonUP = OneButton(UP_BTN_PIN, LOW, false); 
OneButton buttonDN = OneButton(DN_BTN_PIN, LOW, false);

/**
 * App setup
 *
 * @return void
 */
void setup() {
  // INIT PINS
  pinMode(IR_PIN, OUTPUT);
  pinMode(SEL_BTN_PIN, INPUT);
  pinMode(BUZZ_PIN, OUTPUT);

  // INIT DISPLAY
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // My OLED is actually set for 0x3C, not Adafruit's 0x3D
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.cp437(true); // Use full 256 char 'Code Page 437' font

  // INIT BUTTONS
  buttonUP.attachClick(goUp);
  buttonUP.attachDuringLongPress(goUpFast);
  buttonDN.attachClick(goDown);
  buttonDN.attachDuringLongPress(goDownFast);
  
  showHome();
  buzz();
}


/**
 * Format seconds as 0h00m00s
 *
 * @return String
 */
String intervalTime() {
	unsigned int hour = 0;
	unsigned int min = 0;
	unsigned int sec = 0;
  unsigned int time = interval;
  char h = 'h';
  char m = 'm';
  char s = 's';
  String res1, res2;
  
  if (time < 61) {
    res1 = time;
    return res1 + s;
  }

  if (time > 60 && time < 3600) {
    time = time%3600;
    min = time/60;
    time = time%60;
    sec = time;
    res1 = min;
    res2 = sec;

    if (res2.length() == 1) res2 = '0' + res2;

    return res1 + m + res2 + s;
  }

  return "1h0m0s";
}

/**
 * Sound a bleeping alarm for 1 second
 *
 * @return void
 */
void alarm() {
  unsigned int timer = 1000;
  
  while (timer > 0) {
    tone(BUZZ_PIN, 4000);
    delay(50);
    noTone(BUZZ_PIN);
    delay(50);
    timer = timer - 100;
  }
}

/**
 * Sound a buzzer sound for 250 miliseconds
 *
 * @return void
 */
void buzz() {
  tone(BUZZ_PIN, 4000);
  delay(250);
  noTone(BUZZ_PIN);
}

/**
 * Increase current menu page parameter
 * Possible variables: shots, interval, wait
 *
 * @return void
 */
void goUp() {
  switch (page) {
    case 2:
      shots < maxShots && shots++;
      break;
    case 3:
      interval < maxInterval && interval++;
      break;
    case 4:
      wait < maxWait && wait++;
      break;
    default:
      break;
  }

  showPage();
}

/**
 * Fast increase current menu page parameter
 * Possible variables: shots, interval, wait
 *
 * @return void
 */
void goUpFast() {
  switch (page) {
    case 2:
      shots = shots + steps;
      if (shots > maxShots) shots = maxShots;
      break;
    case 3:
      interval = interval + steps;
      if (interval > maxInterval) interval = maxInterval;
      break;
    case 4:
      wait = wait + steps;
      if (wait > maxWait) wait = maxWait;
      break;
    default:
      break;
  }

  showPage();
}

/**
 * Decrease current menu page parameter
 * Possible variables: shots, interval, wait
 *
 * @return void
 */
void goDown() {
  switch (page) {
    case 2:
      shots > minShots && shots--;
      break;
    case 3:
      interval > minInterval && interval--;
      break;
    case 4:
      wait > minWait && wait--;
      break;
    default:
      break;
  }

  showPage();
}

/**
 * Fast decrease current menu page parameter
 * Possible variables: shots, interval, wait
 *
 * @return void
 */
void goDownFast() {
  switch (page) {
    case 2:
      shots = shots - steps;
      if (shots < minShots || shots > maxShots) shots = minShots;
      break;
    case 3:
      interval = interval - steps;
      if (interval < minInterval || interval > maxInterval) interval = minInterval;
      break;
    case 4:
      wait = wait - steps;
      if (wait < minWait || wait > maxWait) wait = minWait;
      break;
    default:
      break;
  }

  showPage();
}

/**
 * Generates the modulating signal and pulses
 * Original code and IR signal hack by Aswan Korula
 * https://bayesianadventures.wordpress.com/2013/08/09/nikon-ml-l3-ir-remote-hack/
 *
 * @param int dCycle
 * @return void
 */
void sendPulse(int dCycle) {
  int iters = dCycle / 23.6;

  for(int i = 0; i <= iters; i++) {
    digitalWrite(IR_PIN, HIGH);
    delayMicroseconds(11);
    digitalWrite(IR_PIN, LOW);
    delayMicroseconds(5);
  }
}

/**
 * Generates a full command signal by repeatedly calling SendPulse()
 * Original code and IR signal hack by Aswan Korula
 * https://bayesianadventures.wordpress.com/2013/08/09/nikon-ml-l3-ir-remote-hack/
 *
 * @return void
 */
void sendSequence() {
  for(int i = 0; i < 2; i++) {
    sendPulse(2000);
    delay(27);
    delayMicroseconds(800);
    sendPulse(500);
    delayMicroseconds(1500);
    sendPulse(500);
    delayMicroseconds(3500);
    sendPulse(500);
    
    if(i < 1) {
      delay(63);
    }
   }
}

/**
 * Commonly repeated display tasks
 *
 * @return void
 */
void setDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
}

/**
 * Present the MAIN page on OLED DISPLAY
 *
 * @return void
 */
void showHome() {
  setDisplay();
  display.println("TAKE SHOTS");
  display.print("Shots:");
  display.println(shots);
  display.print("Del:");

  if (shots == 1) {
    display.println("--");
  } else {
    display.println(intervalTime());
  }

  display.print("Wait:");

  if (wait == 0) {
    display.println("--");
  } else {
    display.print(wait);
    display.println("s");
  }

  display.display();
}

/**
 * Present the SHOTS page on OLED DISPLAY
 *
 * @return void
 */
void showShots() {
  setDisplay();
  display.println("SHOTS:");
  display.println();
  display.setTextSize(3);
  display.println(shots);
  display.display();
}

/**
 * Present the INTERVAL page on OLED DISPLAY
 *
 * @return void
 */
void showInterval() {
  setDisplay();
  display.println("DELAY:");
  display.println();
  display.setTextSize(3);
  display.print(intervalTime());
  display.display();
}

/**
 * Present the DELAY page on OLED DISPLAY
 *
 * @return void
 */
void showWait() {
  setDisplay();
  display.println("WAIT:");
  display.println();
  display.setTextSize(3);
  display.print(wait);
  display.println("s");
  display.display();
}

/**
 * Select and show the current page
 *
 * @return void
 */
void showPage() {
  switch (page) {
    case 1:
      showHome();
      break;
    case 2:
      showShots();
      break;
    case 3:
      showInterval();
      break;
    case 4:
      showWait();
      break;
    default:
      page = 1;
      showHome();
  }
}

/**
 * Countdown before start taking shots
 *
 * @return void
 */
void countDown() {
  unsigned int count = wait;

  if (!wait) return;

  display.setTextSize(5);

  while (count > 0) {
    display.clearDisplay();
    display.setCursor(0, 20);
    display.print(count);
    display.display();

    if (count == 1) {
      alarm();
    } else {
      buzz();
      delay(750);
    }
    
    count--;
  }
}

/**
 * Start shooting sequence
 *
 * @return void
 */
void shoot() {
  countDown();

  for(int i = 1; i <= shots; i++) {
    setDisplay();
    display.println("SHOOTING");
    display.println();
    display.setTextSize(3);
    display.print(i);
    display.print("/");
    display.println(shots);
    display.display();

    sendSequence();
    if (interval > 0) delay(interval * 1000);
  }

  page = 1;
  showHome();
}

/**
 * App loop
 *
 * @return void
 */
void loop() {
  unsigned static long dt;

  buttonUP.tick();
  buttonDN.tick();

  if (digitalRead(SEL_BTN_PIN) == HIGH) {
    if (buttonActive == false) {
			buttonActive = true;
			buttonTimer = millis();
		}

		if ((millis() - buttonTimer > longPressTime) && (longPressActive == false)) {
			longPressActive = true;
      if (page == 1) {
        page++;
      } else {
        page = 1;
      }
      showPage();
		}
	} else {
		if (buttonActive == true) {
			if (longPressActive == true) {
				longPressActive = false;
			} else {
        dt = millis() - buttonTimer;
        if ((dt < longPressTime && dt > shortPressTime) && (longPressActive == false)) {
          if (page == 1) {
            shoot();
          } else {
            page++;
            showPage();
          }
        }
			}
			buttonActive = false;
		}
	}
}
