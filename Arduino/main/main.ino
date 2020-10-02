

#define TEMPERATURE_PIN_NUMBER 5
#define COOL_OUT_PIN 31
#define HEAT_OUT_PIN 33
#define BUTTON_PIN 2

#define DEBUG true

#define RELLAY_SET HIGH
#define RELLAY_OPEN LOW

#include <U8glib.h>
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>


U8GLIB_SSD1306_128X64 display01(U8G_I2C_OPT_NONE);

OneWire oneWireDS(TEMPERATURE_PIN_NUMBER);
DallasTemperature senzorTemper(&oneWireDS);

long int updateTime = 0;

float setPoint = 25.;
float tollPlus = 1.;
float tollMinus = 1.;
float currTemp = -100.;
float highestTemp = 0.;
float lowestTemp = 100.;

bool cooling = false;
bool heating = false;

String stateText = "Starting";


void setup(void) {
  Serial.begin(9600);
  
  senzorTemper.begin();
  pinMode(COOL_OUT_PIN, OUTPUT);
  pinMode(HEAT_OUT_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  byte setPointByte = EEPROM.read(0);
  if(setPointByte < 0xFF) setPoint = setPointByte;

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonClick,  LOW);

  
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

  EEPROM.update(0, (byte)setPoint);
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
  currTemp = senzorTemper.getTempCByIndex(0);

  if(currTemp > highestTemp) highestTemp = currTemp;
  if(currTemp < lowestTemp) lowestTemp = currTemp;

  if(DEBUG){
    Serial.print("Temperature curr: ");
    Serial.println(currTemp);

    Serial.print("Temperature highest: ");
    Serial.println(highestTemp);

    Serial.print("Temperature lowest: ");
    Serial.println(lowestTemp);
  }
  
}

void regulate(void){

  if(currTemp == -127.){
    stateText = "Err sensor disconn";
    cool(false);
    heat(false);
    return;
  }
  
  if(currTemp <= -99.){
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

///////////////////////////////////////
bool isCoolRunning = false;
long coolStartTime = 0;

void cool(bool set){
  if(DEBUG){
    Serial.print("Cool ");
    Serial.println(set);
  }
  
  if(!set){
    isCoolRunning = false;
    digitalWrite(COOL_OUT_PIN, RELLAY_SET);
    return;
  }

  if(!isCoolRunning){
    isCoolRunning = true;
    coolStartTime = millis();
  }

  digitalWrite(COOL_OUT_PIN, RELLAY_OPEN);
}
void heat(bool set){
  if(DEBUG){
    Serial.print("Heat ");
    Serial.println(set);
  }
  
  if(!set)digitalWrite(HEAT_OUT_PIN, RELLAY_SET);
  else digitalWrite(HEAT_OUT_PIN, RELLAY_OPEN);
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
  if(isCoolRunning) {
    display01.print(" ");
    display01.print(timeToStr(millis()-coolStartTime));
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
      result += "h ";
    }
    
    result += minuts;
    result += "m ";
  }
  result += sec;
  result += "s";

  return result;
}
