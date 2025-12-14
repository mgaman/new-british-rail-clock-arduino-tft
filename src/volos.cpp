#include <Arduino.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI()  ;    // Invoke custom library
TFT_eSprite baseSprite = TFT_eSprite(&tft);  // contains circles

void setup() {
    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK); 
  // create base sprite & fill it
  // If I make a sprite the same size as the tft screen my code freezes.
    baseSprite.createSprite(tft.width()-1,tft.height()-1);
    baseSprite.fillSprite(TFT_BLACK);

}
int x=0;
#define C_Y 110
void loop() {
    baseSprite.fillCircle(x,C_Y,30,TFT_BLACK);
    x++;
    if (x > tft.width())
        x = 0;
    baseSprite.fillCircle(x,C_Y,30,TFT_ORANGE);
    baseSprite.fillRect(30,130,100,100,TFT_BLACK);
    baseSprite.drawString(String(x),31,131,7);
    baseSprite.pushSprite(1,1);
  //  delay(100);
}