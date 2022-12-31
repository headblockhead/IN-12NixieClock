/*
  Sketch for the project "Clock on GRI version 2"
  Project page (diagrams, descriptions): https://alexgyver.ru/nixieclock_v2/
  Sources on GitHub: https://github.com/AlexGyver/NixieClock_v2
  Like the way the code is written? Support the author! https://alexgyver.ru/support_alex/
  Author: AlexGyver Technologies, 2018
  https://AlexGyver.ru/
*/

/*
  Control:
  - Time setting:
    - Left button - selection, the rest are "greater" and "less"
    - Hold "select" - time setting
    - Click on "select" - change hour/minute setting
    - Click "more" and "less" - change the time
    - Hold "select" - return to clock mode

  - Effect control IN CLOCK MODE:
    - Holding the central button turns on and off the "glitches"
    - Click on the central button to switch the lighting modes of the lamps
      - Breath
      - Permanent glow
      - Disabled
    - Clicking on the right button toggles the number scrolling modes
      - no effect
      - Smooth fade
      - Rewind in numerical order
      - Rewind on cathodes
      - Train
      - Elastic
*/
/*
  Version 2.0 (together with Pavel Smelov):
  - Fixed jambs with backlight
  - Added switching effects "train" and "gum"
  - Optimized code

  Version 2.1:
  - Fixed a bug with the passage of time

  Version 2.2:
  - Reset seconds when setting time
  
  Version 2.3:
  - Added advanced brightness settings
  - Fixed freezes at zero values ​​of some settings

  Version 2.4:
  - We continue to fix bugs with zero settings

  Version 2.5 ***THIS VERSION***:
  - Demonstration of the effect when selected
*/

// ************************** SETTINGS **************************
#define BOARD_TYPE 0
// clock board type:
// 0 - IN-12 turned (indicators are correct)
// 1 - IN-12 (indicators are upside down)
// 2 - IN-14 (regular and neon dot)
// 3 other indicators

#define DUTY 200        // PWM duty cycle. The voltage depends on the duty cycle! I have 175 volts at 180 and 145 volts at 120

// ======================= EFFECTS ========================
// clock flip effects
byte FLIP_EFFECT = 0;
// The selected one is active at the first start and is changed by buttons. Memorized in memory
// 0 - no effect
// 1 - fade in and out (recommended speed: 100-150)
// 2 - rewind in numerical order (recommended speed: 50-80)
// 3 - rewind in order of the cathodes in the lamp (recommended speed: 30-50)
// 4 - train (recommended speed: 50-170)
// 5 - elastic band (recommended speed: 50-150)


// ======================= BRIGHTNESS =======================
#define NIGHT_LIGHT 1       // Change brightness from time of day (1 on, 0 off)
#define NIGHT_START 23      // hour of transition to night illumination (BRIGHT_N)
#define NIGHT_END 7         // hour of transition to daylight (BRIGHT)

#define INDI_BRIGHT 23      // the brightness of the digits is daytime (1 - 24)! 24 may create burn-in!
#define INDI_BRIGHT_N 8     // night brightness (1 - 24)

#define DOT_BRIGHT 35       // dot brightness daytime (1 - 255)
#define DOT_BRIGHT_N 15     // dot brightness at night (1 - 255)

#define BACKL_BRIGHT 250    // Max. daytime lamp brightness (0 - 255)
#define BACKL_BRIGHT_N 50   // Max. lamp backlight brightness at night (0 - 255, 0 - backlight off)
#define BACKL_MIN_BRIGHT 20 // min. brightness of lamp illumination in breathing mode (0 - 255)
#define BACKL_PAUSE 400     // "darkness" interval between flashes of lamp illumination in breathing mode, ms

// ======================= Glitches =======================
#define GLITCH_MIN 30       // minimum time between glitches, seconds
#define GLITCH_MAX 120      // maximum time between glitches, seconds

// ======================  BLINK =======================
#define DOT_TIME 500        // dot flashing time, ms
#define DOT_TIMER 20        // dot brightness step, ms

#define BACKL_STEP 2        // backlight flashing step
#define BACKL_TIME 5000     // backlight period, ms

// ==================  ANTI BURN-IN ====================
#define BURN_TIME 10        // indicator bypass period in cleaning mode, ms
#define BURN_LOOPS 3        // number of cleaning cycles for each period
#define BURN_PERIOD 15      // anti-burn-in period, minutes


// *********************** FOR DEVELOPERS ***********************
byte BACKL_MODE = 0;                          // The selected one is active at startup and can be changed with buttons
byte FLIP_SPEED[] = {0, 130, 50, 40, 70, 70}; // effects speed, ms (do not change the number)
byte FLIP_EFFECT_NUM = sizeof(FLIP_SPEED);    // number of effects
boolean GLITCH_ALLOWED = 1;                   // 1 - enable, 0 - disable glitches. Button controlled

// --------- ALARM ---------
#define ALM_TIMEOUT 30      // alarm timeout
#define FREQ 900            // alarm frequency

// pins
#define PIEZO 2   // alarm
#define KEY0 3    // hours
#define KEY1 4    // hours
#define KEY2 5    // minutes
#define KEY3 6    // minutes
#define BTN1 7    // button 1
#define BTN2 8    // button 2
#define GEN 9     // generator
#define DOT 10    // point
#define BACKL 11  // backlight
#define BTN3 12   // button 3

// decoder
#define DECODER0 A0
#define DECODER1 A1
#define DECODER2 A2
#define DECODER3 A3

// lamp pinout
#if (BOARD_TYPE == 0)
const byte digitMask[] = {7, 3, 6, 4, 1, 9, 8, 0, 5, 2};   // board decoder mask IN-12-turned (normal numbers)
const byte opts[] = {KEY0, KEY1, KEY2, KEY3};              // order of indicators from left to right
const byte cathodeMask[] = {1, 6, 2, 7, 5, 0, 4, 9, 8, 3}; // cathode order IN-12

#elif (BOARD_TYPE == 1)
const byte digitMask[] = {2, 8, 1, 9, 6, 4, 3, 5, 0, 7};   // board decoder mask IN-12 (numbers upside down)
const byte opts[] = {KEY3, KEY2, KEY1, KEY0};              // order of indicators from right to left (for IN-12 turned) and IN-14
const byte cathodeMask[] = {1, 6, 2, 7, 5, 0, 4, 9, 8, 3}; // cathode order IN-12

#elif (BOARD_TYPE == 2)
const byte digitMask[] = {9, 8, 0, 5, 4, 7, 3, 6, 2, 1};   // IN-14 board decoder mask
const byte opts[] = {KEY3, KEY2, KEY1, KEY0};              // order of indicators from right to left (for IN-12 turned) and IN-14
const byte cathodeMask[] = {1, 0, 2, 9, 3, 8, 4, 7, 5, 6}; // cathode order IN-14

#elif (BOARD_TYPE == 3)
const byte digitMask[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};   // here enter your order of pins
const byte opts[] = {KEY0, KEY1, KEY2, KEY3};              // own order of indicators
const byte cathodeMask[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}; // and its order of cathodes

#endif

/*
  ард ног ном
  А0  7   4
  А1  6   2
  А2  4   8
  А3  3   1
*/
