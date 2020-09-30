

#include "U8glib.h"
#include <OneWire.h>
#include <DallasTemperature.h>


U8GLIB_SSD1306_128X64 display01(U8G_I2C_OPT_NONE);

const int pinTempSensor = 5;
OneWire oneWireDS(pinTempSensor);
DallasTemperature senzorTemper(&oneWireDS);

long int updateTime = 0;

float setPoint = 25.;
float tollPlus = 0.;
float tollMinus = 1.;
float currTemp = -1.;

bool cooling = false;
bool heating = false;

char* stateText = "--XX--";


void setup(void) {
  senzorTemper.begin();
  
  // pro otočení displeje o 180 stupňů
  // stačí odkomentovat řádek níže
  // display01.setRot180();
}

void loop(void) {

  
  if (millis()-updateTime > 500) {

    updateData();
    regulate();
    
    display01.firstPage();
    do {
      printData();
    } while( display01.nextPage() );

    updateTime = millis();
  }
  
  // zde je místo pro další příkazy pro Arduino
  
  // volitelná pauza 10 ms pro demonstraci
  // vykonání dalších příkazů
  delay(10);
}

void updateData(void){
  senzorTemper.requestTemperatures();
  currTemp = senzorTemper.getTempCByIndex(0);
}

void regulate(void){
  
  if(currTemp == -1.){
    stateText = "Initializing";
    cool(false);
    heat(false);
    return;
  }

  if(currTemp > setPoint + tollPlus){
    cool(true);
    heat(false);
    stateText = "Chladi";
    return;
  }

  if(currTemp < setPoint - tollMinus){
    cool(false);
    heat(true);
    stateText = "Hreje";
    return;
  }

  cool(false);
  heat(false);
  stateText = "Teplota OK";
}

void cool(bool set){}
void heat(bool set){}

void printData(void) {
  // nastavení písma, toto písmo umožní vypsat
  // přibližně 15x4 znaků
  display01.setFont(u8g_font_unifont);
  
  // nastavení pozice výpisu v pixelech
  // souřadnice jsou ve tvaru x, y
  // souřadnice 0, 0 je v levém horní rohu
  // OLED displeje, maximum je 128, 64
  display01.setPrintPos(0, 10);
  display01.print("Pozad   ");
  display01.print(setPoint);
  
  display01.setPrintPos(0, 25);
  display01.print("Aktual  ");
  display01.print(currTemp);
  
  display01.setPrintPos(0, 40);
  display01.print(stateText);
  
  display01.setPrintPos(0, 55);
  display01.print("Running ");
  display01.print(millis()/1000);
  
}
