#include <DS1302.h>

// Init the DS1302
DS1302 rtc(2, 3, 4);

int wait = 500; //500ms for 30min fade     // 10ms internal crossFade delay; increase for slower fades
int hold = 500;       // Optional hold when a color is complete, before the next crossFade
int DEBUG = 1;      // DEBUG counter; if set to 1, will write values back via serial
int loopCount = 25; // How often should DEBUG report?
int repeat = 1;     // How many times should we loop before stopping? (0 for no stop)
int j = 0;          // Loop counter for repeat

int setHour = 5; // Set hours to wake (military time)
int setMin = 30; // Set minute to wake

bool started = false;

int sleepHour = 6; // Set hours to wake (military time)
int sleepMin = 15; // Set minute to wake



// Output
int redPin = 9;   // Red LED,   connected to digital pin 9
int grnPin = 10;  // Green LED, connected to digital pin 10
int bluPin = 11;  // Blue LED,  connected to digital pin 11

// Color array
int colorGrid[36][3] = {
  {0, 0, 0 },
  {3,  0,  0},
  {6,  0,  0},
  {9,  0,  0},
  {12, 0,  0},
  {15, 0,  0},
  {18, 0,  0},
  {21, 0,  0},
  {24, 0,  0},
  {27, 0,  0},
  {30,  0,  0},
  {32,  5,  0},
  {34,  10, 0},
  {36,  15, 0},
  {38,  20, 0},
  {40,  25, 0},
  {42,  28, 0},
  {44,  31, 0},
  {46,  34, 0},
  {48,  37, 0},
  {50,  40, 0},
  {52,  42, 6},
  {54,  44, 12},
  {56,  46, 18},
  {58,  48, 24},
  {60,  50, 30},
  {64,  56, 40},
  {68,  62, 50},
  {72,  68, 60},
  {76,  74, 70},
  {80,  80, 80},
  {84,  84, 84},
  {88,  88, 88},
  {92,  92, 92},
  {96,  96, 96},
  {100, 100,  100}
};
// etc.

// Set initial color
int redVal = colorGrid[0][0];
int grnVal = colorGrid[0][1]; 
int bluVal = colorGrid[0][2];

// Initialize color variables
int prevR = redVal;
int prevG = grnVal;
int prevB = bluVal;

Time t;

// Set up the LED outputs
void setup()
{  
  
//run once, to set time
//  rtc.setDOW(FRIDAY);        // Set Day-of-Week to FRIDAY
//  rtc.setTime(18, 23, 0);    // Set the time to 12:00:00 (24hr format)
//  rtc.setDate(11, 1, 2019); // Set the date to date,month,year
  
  pinMode(redPin, OUTPUT);   // sets the pins as output
  pinMode(grnPin, OUTPUT);   
  pinMode(bluPin, OUTPUT); 

  if (DEBUG) {           // If we want to see values for debugging...
    Serial.begin(9600);  // ...set up the serial ouput 
  }
}

// Main program: list the order of crossfades
void loop()
{
  t = rtc.getTime(); // Make a time class called 't'
  
//  // Send Day-of-Week
//  Serial.print(rtc.getDOWStr());
//  Serial.print(" ");
//  
//  // Send date
//  Serial.print(rtc.getDateStr());
//  Serial.print(" -- ");

Serial.print(colorGrid[1][0]);

  // Send time
  Serial.println(rtc.getTimeStr());

  if (t.hour == setHour && t.min == setMin && started == false) // Check if it's time to wake up!
  {
    alarm();
  }
  else if (t.hour == sleepHour && t.min == sleepMin && started == true)
  {
    reset();
  }
  
  // Wait one second before repeating
  delay (1000);
}

void alarm()
{
  started = true;
  for (int i = 1; i < 36; i++) {
    Serial.print("Swath:  ");
    Serial.print(colorGrid[i][0]);
    Serial.print(" | ");
    Serial.print(colorGrid[i][1]);
    Serial.print(" | ");
    Serial.print(colorGrid[i][2]);
    crossFade(colorGrid[i]);
  }
}

void reset()
{
  started = false;
  redVal = colorGrid[0][0];
  grnVal = colorGrid[0][1]; 
  bluVal = colorGrid[0][2];

  // reset color variables
  prevR = redVal;
  prevG = grnVal;
  prevB = bluVal;
  analogWrite(redPin, 0);
    analogWrite(grnPin, 0);      
    analogWrite(bluPin, 0); 
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
    step = 100/step;              //   divide by 1020
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

void crossFade(int color[3]) {
  Serial.println(rtc.getTimeStr());
  // Convert to 0-255
  int R = (color[0] * 255) / 100;
  int G = (color[1] * 255) / 100;
  int B = (color[2] * 255) / 100;

  int stepR = calculateStep(prevR, R);
  int stepG = calculateStep(prevG, G); 
  int stepB = calculateStep(prevB, B);

  for (int i = 0; i <= 100; i++) {
    redVal = calculateVal(stepR, redVal, i);
    grnVal = calculateVal(stepG, grnVal, i);
    bluVal = calculateVal(stepB, bluVal, i);

    analogWrite(redPin, redVal);   // Write current values to LED pins
    analogWrite(grnPin, grnVal);      
    analogWrite(bluPin, bluVal); 

    delay(wait); // Pause for 'wait' milliseconds before resuming the loop

    if (DEBUG) { // If we want serial output, print it at the 
      if (i == 0 or i % loopCount == 0) { // beginning, and every loopCount times
        Serial.print("Loop/RGB: #");
        Serial.print(i);
        Serial.print(" | ");
        Serial.print(redVal);
        Serial.print(" / ");
        Serial.print(grnVal);
        Serial.print(" / ");  
        Serial.println(bluVal); 
      } 
      DEBUG += 1;
    }
  }
  // Update current values for next loop
  prevR = redVal; 
  prevG = grnVal; 
  prevB = bluVal;
  delay(hold); // Pause for optional 'wait' milliseconds before resuming the loop
}
