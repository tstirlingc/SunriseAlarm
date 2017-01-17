#ifndef EASTER_EGG_HPP
#define EASTER_EGG_HPP

namespace SunriseAlarmEasterEgg {
  
#define EASTER_EGG_MINUTES 5
#define EASTER_EGG_ENTRY_WINDOW 1000
#define EASTER_EGG_NUM_TOUCH_TO_ENTER 5

int default_brightness = 255; // 30 is nice

SunriseAlarm::ClockTime easterEggStartTime;
SunriseAlarm::ClockTime easterEggEndTime;

bool isTimeToEndEasterEgg() {
  SunriseAlarm::ClockTime current = SunriseAlarm::current_clock_time;
  return !SunriseAlarm::isTimeInWindow(SunriseAlarm::current_clock_time, easterEggStartTime, easterEggEndTime);
}

int numTouched = 0;
uint32_t easterEggEntryStarted = 0;

bool easterEggEntered() {
  if (SunriseAlarm::touchSensorTouched()) {
    if (numTouched == 0) easterEggEntryStarted = millis();
    ++numTouched;
    SunriseAlarm::waitForTouchSensorDepress();
    if (numTouched >= EASTER_EGG_NUM_TOUCH_TO_ENTER) {
      numTouched = 0;
      return true;
    }
  }
  if (SunriseAlarm::isTimeNow(easterEggEntryStarted, EASTER_EGG_ENTRY_WINDOW)) numTouched = 0;
  return false;
}

void adjustBrightness() {
  int16_t encoderDelta = SunriseAlarm::encoder->getValue();
  if (encoderDelta != 0) {
    default_brightness += encoderDelta;
    default_brightness = min(255,max(1,default_brightness));
    SunriseAlarm::strip.setBrightness(default_brightness); 
  }
}

int calculateStep(int prevValue, int endValue);

// Set initial color
int redVal = 0;
int grnVal = 0; 
int bluVal = 0;

int wait = 1;      // 10ms internal crossFade delay; increase for slower fades
int holdBeforeNextCrossFade = 250;       // Optional hold when a color is complete, before the next crossFade

// Initialize color variables
int prevR = redVal;
int prevG = grnVal;
int prevB = bluVal;

int color[3];
int stepR;
int stepG; 
int stepB;

void crossFadeSetup() {
  SunriseAlarm::strip.setBrightness(default_brightness); 
  for (int i=0 ; i<NUM_LED ; ++i) {
    SunriseAlarm::strip.setPixelColor(i,prevR,prevG,prevB);
  }
  SunriseAlarm::strip.show();
  color[0] = random(256);
  color[1] = random(256);
  color[2] = random(256);
  stepR = calculateStep(prevR, color[0]);
  stepG = calculateStep(prevG, color[1]); 
  stepB = calculateStep(prevB, color[2]);
}


/* BELOW THIS LINE IS THE MATH -- YOU SHOULDN'T NEED TO CHANGE THIS FOR THE BASICS
* 
* The program works like this:
* Imagine a crossfade that moves the red LED from 0-10, 
*   the green from 0-5, and the blue from 10 to 7, in
*   ten steps.
*   We'd want to count the 10 steps and increase or 
*   decrease color values in evenly stepped increments.
*   Imagine a + indicates raising a value by 1, and a -
*   equals lowering it. Our 10 step fade would look like:
* 
*   1 2 3 4 5 6 7 8 9 10
* R + + + + + + + + + +
* G   +   +   +   +   +
* B     -     -     -
* 
* The red rises from 0 to 10 in ten steps, the green from 
* 0-5 in 5 steps, and the blue falls from 10 to 7 in three steps.
* 
* In the real program, the color percentages are converted to 
* 0-255 values, and there are 1020 steps (255*4).
* 
* To figure out how big a step there should be between one up- or
* down-tick of one of the LED values, we call calculateStep(), 
* which calculates the absolute gap between the start and end values, 
* and then divides that gap by 1020 to determine the size of the step  
* between adjustments in the value.
*/

int calculateStep(int prevValue, int endValue) {
  int step = endValue - prevValue; // What's the overall gap?
  if (step) {                      // If its non-zero, 
    step = 1020/step;              //   divide by 1020
  } 
  return step;
}

/* The next function is calculateVal. When the loop value, i,
*  reaches the step size appropriate for one of the
*  colors, it increases or decreases the value of that color by 1. 
*  (R, G, and B are each calculated separately.)
*/

int calculateVal(int step, int val, int i) {

  if ((step) && i % step == 0) { // If step is non-zero and its time to change a value,
    if (step > 0) {              //   increment the value if step is positive...
      val += 1;           
    } 
    else if (step < 0) {         //   ...or decrement it if step is negative
      val -= 1;
    } 
  }
  // Defensive driving: make sure val stays in the range 0-255
  if (val > 255) {
    val = 255;
  } 
  else if (val < 0) {
    val = 0;
  }
  return val;
}

/* crossFade() converts the percentage colors to a 
*  0-255 range, then loops 1020 times, checking to see if  
*  the value needs to be updated each time, then writing
*  the color values to the correct pins.
*/

int crossFadeIndex = 0;
uint32_t lastHoldStarted = 0;

void crossFadeToRandomColor() {
  if (!SunriseAlarm::isTimeNow(lastHoldStarted, holdBeforeNextCrossFade))  return;
  
  {
    redVal = calculateVal(stepR, redVal, crossFadeIndex);
    grnVal = calculateVal(stepG, grnVal, crossFadeIndex);
    bluVal = calculateVal(stepB, bluVal, crossFadeIndex);

    for (int pixelI=0 ; pixelI<NUM_LED ; ++pixelI) {
      SunriseAlarm::strip.setPixelColor(pixelI, redVal, grnVal, bluVal);
    }
    SunriseAlarm::strip.show();
    delay(wait); // Pause for 'wait' milliseconds before resuming the loop
  }
  // Update current values for next loop
 // Pause for optional 'wait' milliseconds before resuming the loop

  ++crossFadeIndex;
  lastHoldStarted = millis()-holdBeforeNextCrossFade-1;
  if (crossFadeIndex > 1020) {
    crossFadeIndex = 0;
    prevR = redVal;
    prevG = grnVal;
    prevB = bluVal;
    color[0] = random(256);
    color[1] = random(256);
    color[2] = random(256);
    stepR = calculateStep(prevR, color[0]);
    stepG = calculateStep(prevG, color[1]); 
    stepB = calculateStep(prevB, color[2]);
    lastHoldStarted = millis();
  }
}

} // namespace SunriseAlarmEasterEgg

#endif // EASTER_EGG_HPP

