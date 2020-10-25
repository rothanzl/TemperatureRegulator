

#define TEMPERATURE_PIN_NUMBER 5
#define COOL_OUT_PIN 31
#define HEAT_OUT_PIN 33
#define BUTTON_PIN 2

#define DEBUG true

#define HEAT_ENABLE true
#define COOL_ENABLE true

#define RELLAY_SET LOW
#define RELLAY_OPEN HIGH

#include <U8glib.h>
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>


U8GLIB_SSD1306_128X64 display01(U8G_I2C_OPT_NONE);

OneWire oneWireDS(TEMPERATURE_PIN_NUMBER);
DallasTemperature senzorTemper(&oneWireDS);

long int updateTime = 0;

float setPoint = 20.;

float tollPlus = 1.;
float tollPlusOff = 0.5;

float tollMinus = 1.;
float tollMinusOff = -0.75;
float currTemp = -100.;
float prevTemp = -100.;
float highestTemp = 0.;
float lowestTemp = 100.;

bool isCooling = false;
bool isHeating = false;

String stateText = "Starting";


void setup(void) {
  Serial.begin(9600);
  
  senzorTemper.begin();
  pinMode(COOL_OUT_PIN, OUTPUT);
  cool(false);
  
  pinMode(HEAT_OUT_PIN, OUTPUT);
  heat(false);

  
  //pinMode(BUTTON_PIN, INPUT_PULLUP);
  //byte setPointByte = EEPROM.read(0);
  //if(setPointByte < 0xFF) setPoint = setPointByte;
  //attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonClick,  LOW);

  
  // pro otočení displeje o 180 stupňů
  // stačí odkomentovat řádek níže
  // display01.setRot180();
}

long lastButtonClick = 0;
#define BUTTON_TIME_INSENSITIVITY 50
void buttonClick(){
  if(lastButtonClick + BUTTON_TIME_INSENSITIVITY > millis()) return;
  
  lastButtonClick = millis();
  setPoint++;
  if(setPoint > 30) setPoint = 0;

  //EEPROM.update(0, (byte)setPoint);
  printDisplay();
  
  if(DEBUG){
    Serial.print("Set point ");
    Serial.println(setPoint);
  }
}

void loop(void) {

  
  if (millis()-updateTime > 500) {

    updateTemperData();
    regulate();
    printDisplay();

    updateTime = millis();
  }
  
}

void updateTemperData(void){
  senzorTemper.requestTemperatures();

  float temp = senzorTemper.getTempCByIndex(0);
  if(temp != currTemp){
    prevTemp = currTemp;
    currTemp = temp;
  }
  

  if(currTemp > highestTemp) highestTemp = currTemp;
  if(currTemp < lowestTemp) lowestTemp = currTemp;

  if(DEBUG){
    Serial.print("Temperature curr: ");
    Serial.print(currTemp);

    Serial.print(" highest: ");
    Serial.print(highestTemp);

    Serial.print(" lowest: ");
    Serial.println(lowestTemp);
  }
  
}

void regulate(void){

  if(currTemp == -127.){
    stateText = "Err sensor disconn";
    if(DEBUG) Serial.println("Error sensor disconnect");
    cool(false);
    heat(false);
    return;
  }
  
  if(currTemp <= -99.){
    stateText = "Initializing";
    if(DEBUG) Serial.println("Initializing");
    cool(false);
    heat(false);
    return;
  }

  if(COOL_ENABLE){
    // start cool
    if(currTemp >= setPoint + tollPlus && !isCooling){
      if(DEBUG) Serial.println("Start cool");
      cool(true);
      heat(false);
      stateText = "Chl";
      return;
    }
  
    // stop cool
    if(currTemp <= setPoint + (tollPlus * tollPlusOff) && isCooling){
      if(DEBUG) Serial.println("Stop cool");
      cool(false);
      heat(false);
    }
  
    // continue cool
    if(currTemp > setPoint && isCooling){
      return;
    }
  }

  
  
  if(HEAT_ENABLE){
    // start heat
    if(currTemp <= setPoint - tollMinus && !isHeating){
      if(DEBUG) Serial.println("Start heat");
      cool(false);
      heat(true);
      stateText = "Top";
      return;
    }
  
  
    // stop heat
    if(currTemp >= setPoint - (tollMinus * tollMinusOff) && isHeating){
      if(DEBUG) Serial.println("Stop heat");
      cool(false);
      heat(false);
    }
  
    // continue heat
    if(currTemp < setPoint && isHeating){
      return;
    }
  }
  
  

  

  stateText = "Teplota OK";
}

///////////////////////////////////////
long coolStartTime = 0;

void cool(bool set){
  if(DEBUG){
    Serial.print("Cool ");
    Serial.println(set);
  }
  
  if(set){
    if(!isCooling) coolStartTime = millis();
    isCooling = true;
    digitalWrite(COOL_OUT_PIN, RELLAY_SET);
  }else{
    isCooling = false;
    digitalWrite(COOL_OUT_PIN, RELLAY_OPEN);
  }

}

long heatStartTime = 0;
void heat(bool set){
  if(DEBUG){
    Serial.print("Heat ");
    Serial.println(set);
  }
  
  if(set){
    if(!isHeating) heatStartTime = millis();
    digitalWrite(HEAT_OUT_PIN, RELLAY_SET);
    isHeating = true;
  }
  else {
    digitalWrite(HEAT_OUT_PIN, RELLAY_OPEN);
    isHeating = false;
  }
}

void printDisplay(){
  display01.firstPage();
  do {
    printDisplayPage();
  } while( display01.nextPage() );
}

void printDisplayPage() {
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

  if(prevTemp < currTemp) display01.print(" St");
  if(prevTemp > currTemp) display01.print(" Kl");
  
  if(isCooling) {
    display01.print(" ");
    display01.print(timeToStr(millis()-coolStartTime));
  }
  if(isHeating){
    display01.print(" ");
    display01.print(timeToStr(millis()-heatStartTime));
  }
  
  display01.setPrintPos(0, 55);
  display01.print("Running ");
  display01.print(timeToStr(millis()));
  
}

String timeToStr(long timeMillis){
  long sec = timeMillis / 1000;
  String result = "";
  if(sec > 60){
    long minuts = sec / 60;
    sec -= (minuts*60);

    if(minuts > 60){
      long hours = minuts / 60;
      minuts -= (hours * 60);
      result += hours;
      result += ":";
    }else{
      result += "0:";
    }
    
    result += minuts;
    result += ":";
  }else{
    result += "0:0:";
  }
  result += sec;

  return result;
}
