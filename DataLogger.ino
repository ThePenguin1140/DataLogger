#include <DHT.h> //Temp sensor
#include <SPI.h> // File System
#include <SD.h>  // SD Card 
#include <LiquidCrystal.h>  //Screen
#include <DS1302.h> //Clock

#define RST 2
#define DAT 3
#define CLK 5

#define DHTPIN A5
#define DHTTYPE DHT11
#define INDICATOR 6
#define LIGHT A0
#define BTN 9
#define RS 7
#define E 8
#define D4 A1
#define D5 A2
#define D6 A3
#define D7 A4

boolean setClock = false;

DS1302 rtc(RST, DAT, CLK);
LiquidCrystal lcd(RS, E, D4, D5, D6, D7);
DHT dht(DHTPIN, DHTTYPE);

char displayContent[2][16];
int counter = 0;
int displayState = 0;
boolean recording = false;
Time lastReadingTime(0,0,0,0,0,0,Time::kSunday);
String lastReading[3];
String fileName;

void setup() {
  Serial.begin(9600);

  //configure ethernet shield
  Serial.print("Disable Ethernet...");
  pinMode(10, OUTPUT);
  Serial.print("...");
  digitalWrite(10, HIGH);
  Serial.println("DONE");

  //init SD
  Serial.print("Initializing SD card communications...");
  if ( !SD.begin(4) ) {
    Serial.println("FAILED");
    return;
  }
  Serial.println("DONE");

  //Init Log file
  Serial.print("Searching for log file...");
  Time t = rtc.time();
  fileName = String(t.yr) + "-" + String(t.mon) + "-" + String(t.date) + "-DataLog.csv";
  Serial.print("file name: " + fileName + "...");
  if ( SD.exists("test.csv") ) {
    Serial.println("FOUND");
    Serial.print("Removing file...");
    SD.remove("test.csv");
    Serial.println("DONE");
  }else{
    Serial.println("DONE");
  }
  
  Serial.print("Creating new Log File...");
  File logFile = SD.open("test.csv", FILE_WRITE);
  Serial.print("...");
  if (logFile) {
    Serial.println("DONE");
    logFile.close();
  } else {
    Serial.println("ERROR");
  }

  //config various pins
  Serial.print("Setting pin modes...");
  pinMode(BTN, INPUT);
  Serial.print("...");
  pinMode(INDICATOR, OUTPUT);
  Serial.println("DONE");

  //set clock if selected
  if (setClock) {
    Serial.print("Setting Clock...");
    rtc.halt(false);
    rtc.writeProtect(false);
    Time t(2015, 10, 9, 12, 00, 00, Time::kFriday);
    rtc.time(t);
    Serial.println("DONE");
  }

  counter = 0;

  //init display
  lcd.begin(16,2);
  Serial.print("Initializing Display...");
  for( int i = 0; i < 2; i++) {
    for( int n = 0; n < 17; n++) {
      displayContent[i][n] = '-';
    }
  }
  push("IDLE");
  push("PUSH BTN TO STRT");
  Serial.println("DONE");
  Serial.println("SETUP COMPLETE");
}

void loop() {
  //check if the button is pressed and change
  int btn = digitalRead(BTN);
  //the state of the system accordingly
  if (btn && recording) {
    recording = false;
    digitalWrite(INDICATOR, LOW);
    push(createDisplayTimeStamp(rtc.time()));
    push("COMPLETE");
  } else if (btn && !recording) {
    recording = true;
    digitalWrite(INDICATOR, HIGH);
    push("Starting...");
  }

  //ever 10 seconds  
  if (millis()%10000==0) {
    //every 5 minutes if we're recording
    if ( recording && millis()%300000==0) {
      //write a reading to the log file
      recordReading();
      Serial.println(constructLogEntry());
      counter++;
      lastReadingTime = rtc.time();
    } else if ( recording ) {
      //otherwise just update the screen
      displayState = alternateActiveDisplay(displayState);
    }
  }

  //every half second update the time if it's the
  //current display on the unit
  if (millis()%500==0 && !displayState) {
    if (recording) {
      updateActiveTime();
    }
  }

  //Serial.println(constructLogEntry());
}

void updateActiveTime() {
  push(createDisplayTimeStamp(rtc.time()));
  push(createDisplayTimeStamp(lastReadingTime));
}

String createDisplayTimeStamp(Time t) {
  return
    makeDoubleDigit(t.yr - 2000) +
    makeDoubleDigit(t.mon) +
    makeDoubleDigit(t.date) +
    " " +
    makeDoubleDigit(t.hr) +
    makeDoubleDigit(t.min) +
    makeDoubleDigit(t.sec);
}

String createLogTimeStamp(Time t) {
  return
    String(t.yr) +
    "-" +
    intToMonth(t.mon) +
    "-" +
    makeDoubleDigit(t.date) +
    " " +
    makeDoubleDigit(t.hr) +
    ":" +
    makeDoubleDigit(t.min) +
    ":" +
    makeDoubleDigit(t.sec);

}

int alternateActiveDisplay(int state) {
  if (state) {
    //show idle + msg
    push("LS# " + counter);
    push(
      "T " + lastReading[0] +
      "H " + lastReading[1] +
      "L " + lastReading[2]
    );
    state = 0;
  } else {
    //show idle + time
    updateActiveTime();
    state = 1;
  }
  return state;
}

String readTemp() {
  return String( (int) ( dht.readTemperature() + 0.5 ) );
}

String readHumidity() {
  return String( (int) ( dht.readHumidity() + 0.5 ) );
}

String readLight() {
  return String( (int) ( analogRead(LIGHT) + 0.5 ) );
}

String constructLogEntry() {
  return String(counter) + ", " +
  createLogTimeStamp(rtc.time()) + ", " +
  readTemp() + ", " + readHumidity() + ", " + readLight();
}

void recordReading() {
  String line = constructLogEntry();
  lastReading[0] = readTemp();
  lastReading[1] = readHumidity();
  lastReading[2] = readLight();
  File logFile = SD.open("test.csv");
  if (logFile) {
    Serial.println(line);
    logFile.println(line);
    logFile.close();
  } else {
    Serial.println("ERROR PRINTING TO FILE");
    return;
  }
}

void push(String text) {
  String(displayContent[1]).toCharArray( displayContent[0], 16 );
  //if ( sizeof(text)/sizeof(char)<16 ) {
  //  text = bufferString(text, '*');
  //}
  text.toCharArray( displayContent[1], 16 );

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(displayContent[0]);
  lcd.setCursor(0, 1);
  lcd.write(displayContent[1]);
}

String copyString(char arr[]) {
  int arrLength = sizeof(arr) / sizeof(char);
  char copy[arrLength];


  for (int i = 0; i < arrLength; i++ ) {
    copy[i] = arr[i];
  }
  return copy;
}

String bufferString(String text, char bufferChar) {
  int textLength = sizeof(text) / sizeof(char);
  int space = 16 - textLength; //REMARK might be off by one
  String output = "";

  for (int i = 0; i < space / 2; i++) {
    output += String(bufferChar);
  }
  output += text;
  for (int i = 0; i < space / 2; i++) {
    output += String(bufferChar);
  }
  if (space % 2) output += String(bufferChar);
  return output;
}

String createTimeStampString(int year, int month, int day, int hour, int minute, int second) {
  String dateStr = String(year - 2000);
  dateStr += "-" + intToMonth(month) + "-" + String(day);
  String timeStr = String(hour) + ":" + String(minute) + ":" + String(second);
  return dateStr + " " + timeStr;
}

String makeDoubleDigit(int i) {
  if (i > 9) {
    return String(i);
  } else {
    String d = "0" + String(i);
    return d;
  }
}

String intToMonth(int m) {
  switch (m) {
    case 1:
      return "JAN";
    case 2:
      return "FEB";
    case 3:
      return "MAR";
    case 4:
      return "APR";
    case 5:
      return "MAY";
    case 6:
      return "JUN";
    case 7:
      return "JUL";
    case 8:
      return "AUG";
    case 9:
      return "SEP";
    case 10:
      return "OCT";
    case 11:
      return "NOV";
    case 12:
      return "DEC";
    default:
      return "ERR";
  }
}

