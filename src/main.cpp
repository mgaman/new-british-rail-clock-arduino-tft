/*
   https://www.youtube.com/watch?v=k9GNMepc9xs  
   https://youtube.com/shorts/Q88IBHg2Aqc

*/

#include <Arduino.h>
#include <math.h>
#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library

#define BASE_COLOR TFT_BLACK

#define CIRCLE_COLOR TFT_RED
#define CIRCLE_THICKNESS 4

#define ARROW_WIDTH 10
#define BASE_BORDER 1   // gap around base sprite and screen so base is 239x239
#define DOT_STEPS 480  // steps per 360 degrees i.e. up/down 240 pixels

TFT_eSPI tft = TFT_eSPI()  ;    // Invoke custom library
TFT_eSprite baseSprite = TFT_eSprite(&tft);  // contains circles
TFT_eSprite GtArrowSprite = TFT_eSprite(&tft);  // > arrow
TFT_eSprite LtArrowSprite = TFT_eSprite(&tft);  // < arrow
TFT_eSprite timeSprite = TFT_eSprite(&tft);  // time as text
hw_timer_t *Timer0_Cfg = NULL;

unsigned int sWidth,sHeight,currentMinute;
bool timerTicked = false;
unsigned int OUTER_CIRCLE_INNER_RADIUS,OUTER_CIRCLE_OUTER_RADIUS,CURRENT_STEP;
unsigned int INNER_CIRCLE_INNER_RADIUS,INNER_CIRCLE_OUTER_RADIUS;
float hypotenuse;

void IRAM_ATTR Timer0_ISR()
{
    timerTicked = true;
}

/**
 * @brief Draw both arrows in the same color as the rings. The containing sprite is twice as high as
 * it is wide. For the > shaped arrow, 2 sloped lines are drawn, from (0,0) to (width,width) and (width,width)
 * to (0,width*2). The < shaped arrow is drawn as a mirror of <. The for loop is to make the line thicker.
 * 
 */
void drawArrowSprite() {
  GtArrowSprite.fillSprite(TFT_BLACK);
  LtArrowSprite.fillSprite(TFT_BLACK);
  for (int l=0;l<5;l++) {
    GtArrowSprite.drawLine(0,l,            ARROW_WIDTH+l,ARROW_WIDTH, CIRCLE_COLOR); 
    LtArrowSprite.drawLine(0,l+ARROW_WIDTH,ARROW_WIDTH+l,2*ARROW_WIDTH,CIRCLE_COLOR);
    GtArrowSprite.drawLine(ARROW_WIDTH-l, ARROW_WIDTH,0,(2*ARROW_WIDTH)-l,CIRCLE_COLOR);
    LtArrowSprite.drawLine(ARROW_WIDTH-l, 0          ,0,ARROW_WIDTH-l,CIRCLE_COLOR);
  }
}

/**
 * @brief Format the elasped time as the string HH:MM and draw it into timeSprite
 * 
 * @param time Current time in seconds
 */
void displayTime(unsigned int time) {
  unsigned minutes = time % 60;
  unsigned hours = time / 60;
  hours %= 24;   // limit to 23 hours
  char timeString[10];
  sprintf(timeString,"%02d:%02d",hours,minutes);
  timeSprite.fillSprite(TFT_BLACK);
  timeSprite.drawString(timeString,0,0,6);
}

/**
 * @brief Draw the base sprite which was overwritten by other sprites every timer tick.
 * Paint the sprite in its background color then draw 2 concentric circles
 * 
 */
void drawBase() {  
  baseSprite.fillSprite(BASE_COLOR);
  baseSprite.drawArc(baseSprite.width()/2,baseSprite.height()/2,OUTER_CIRCLE_OUTER_RADIUS,OUTER_CIRCLE_INNER_RADIUS,0,360,CIRCLE_COLOR,BASE_COLOR,true);
  baseSprite.drawArc(baseSprite.width()/2,baseSprite.height()/2,INNER_CIRCLE_OUTER_RADIUS,INNER_CIRCLE_INNER_RADIUS,0,360,CIRCLE_COLOR,BASE_COLOR,true);
}

/**
 * @brief Calculate where to place the 2 arrow sprites on the base sprite. This is calculated relative to
 * the center of the base sprite. The first half calculates the position of the > arrow on the outer circle,
 * the second half the < arrow on the inner circle.
 * Note how I make > go clockwize and < anti-clockwize 
 * 
 * @param angle The angle between 12 and current second, expressed in radians. e.g 15 seconds is at 90 degrees
 * or PI/2 radians. 
 */
void locateArrows(float angle) {
  // calculate pivot points for locating arrows
  int16_t xOuter = hypotenuse*sin(angle);
  int16_t yOuter = hypotenuse*cos(angle);
  // xOuter,yOuter calculated from center of baseSprite, correct for origin in top left
  baseSprite.setPivot((baseSprite.width()/2)+xOuter,(baseSprite.width()/2)-yOuter);
  GtArrowSprite.pushRotated(&baseSprite,(int16_t)(angle*180/PI),TFT_BLACK);  // rotate clockwize
  // repeat for 2nd arrow going anticlockwize
  int16_t xInner = (hypotenuse-ARROW_WIDTH-(CIRCLE_THICKNESS/2)-1)*sin(angle);
  int16_t yInner = (hypotenuse-ARROW_WIDTH-(CIRCLE_THICKNESS/2)-1)*cos(angle);
  baseSprite.setPivot((baseSprite.width()/2)-xInner,(baseSprite.width()/2)-yInner);
  LtArrowSprite.pushRotated(&baseSprite,360-(int16_t)(angle*180/PI),TFT_BLACK);
}

/**
 * @brief Arduino initialization code.
 * There is no real time clock so the time displayed is entirely fake. 
 * Initialise it to whatever you like (currentMinute)
 * 
 */
void setup(void) {
  Serial.begin(115200);
  delay(1000);
  // calculate some values
  sWidth = tft.width();
  sHeight = tft.height();
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(BASE_COLOR); 
  // create base sprite & fill it
  // If I make a sprite the same size as the tft screen my code freezes.
  baseSprite.createSprite(sWidth-(BASE_BORDER*2),sHeight-(BASE_BORDER*2));
  // calculate inner,outer radii of the 2 circles
  OUTER_CIRCLE_OUTER_RADIUS = (baseSprite.width()/2) - ARROW_WIDTH;
  OUTER_CIRCLE_INNER_RADIUS = OUTER_CIRCLE_OUTER_RADIUS - CIRCLE_THICKNESS;
  // place inner circle ARROW_WIDTH inside outer circle
  INNER_CIRCLE_OUTER_RADIUS = OUTER_CIRCLE_INNER_RADIUS - ARROW_WIDTH;
  INNER_CIRCLE_INNER_RADIUS = INNER_CIRCLE_OUTER_RADIUS - CIRCLE_THICKNESS;
  // calculate distance from center to outer circle
  hypotenuse = (OUTER_CIRCLE_INNER_RADIUS+OUTER_CIRCLE_OUTER_RADIUS)/2.0f;
  //Serial.printf("OR %d IR %d hyp %f\r\n",OUTER_CIRCLE_INNER_RADIUS,OUTER_CIRCLE_OUTER_RADIUS,hypotenuse);
  drawBase(); 
  // create  arrow sprites and draw on the circle
  GtArrowSprite.createSprite(ARROW_WIDTH+1,ARROW_WIDTH*2+1);
  LtArrowSprite.createSprite(ARROW_WIDTH+1,ARROW_WIDTH*2+1);
  drawArrowSprite();  // only need to do this once
  // push base to screen
  baseSprite.pushSprite(BASE_BORDER,BASE_BORDER);  // do not ignore background color
  // create sprite for time display, just guessed the size
  timeSprite.createSprite(125,40);
  timeSprite.setTextColor(TFT_WHITE);

  CURRENT_STEP = 0;
  // set timer to interrupt DOT_STEPS times every 60 secs
  Timer0_Cfg = timerBegin(0, 80, true);
  timerAttachInterrupt(Timer0_Cfg, &Timer0_ISR, true);
  timerAlarmWrite(Timer0_Cfg, 60000000L/DOT_STEPS, true);
  timerAlarmEnable(Timer0_Cfg);

  currentMinute = 58;  // want to see minute/hour rollover quickly
  displayTime(currentMinute++);
}

/**
 * @brief Called every clock tick to redraw the screen
 * 
 */
void drawScreen() {
  // place at correct angle
  float currentAngle = (PI*CURRENT_STEP*360.0f/DOT_STEPS)/180.0f;  // angle in radians
  drawBase();
  locateArrows(currentAngle);
  CURRENT_STEP++;
  if (CURRENT_STEP == DOT_STEPS) {
    CURRENT_STEP = 0;
    displayTime(currentMinute++);
  }
  timeSprite.pushToSprite(&baseSprite,55,100);
  // push base to screen
  baseSprite.pushSprite(BASE_BORDER,BASE_BORDER);  // do not ignore background color
}

/**
 * @brief Infinite loop driven by the timer tick. Every time tick redraw the screen
 * 
 */
void loop() {
    if (timerTicked) {
      drawScreen();
      timerTicked = false;
    }
}


