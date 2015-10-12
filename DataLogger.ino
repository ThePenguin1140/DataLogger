#include <DHT.h> //Temp sensor
#include <SPI.h> // File System
#include <SD.h>  // SD Card 
#include <LiquidCrystal.h>  //Screen
#include <DS1302.h> //Clock

#define RST 2
#define DAT 3
#define CLK 4

#define DHTPIN 5
#define DHTTYPE DHT11
#define INDICATOR 6
#define LIGHT A0
#define BTN 13
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
int counter;
int setClock = true;
boolean recording = false;

void setup() {
    Serial.begin(9600);

    Serial.print("Disable Ethernet...");
    pinMode(10, OUTPUT);
    Serial.print("...");
    digitalWrite(10, HIGH);
    Serial.println("DONE");

    Serial.print("Initializing SD card communications...");
    if ( !SD.begin(4) ) {
        Serial.println("FAILED");
        return;
    }
    Serial.println("DONE");

    Serial.print("Searching for log file...");
    Time t = rtc.time();
    String fileName = String(t.yr) + "-" + String(t.mon) + "-" + String(t.date) + "-DataLog.csv";
    Serial.print("file name: " + fileName + "...");
    if ( SD.exists(fileName) ) {
        Serial.println("FOUND");
        Serial.print("Removing file...");
        SD.remove(fileName);
        Serial.println("DONE");
    }
    Serial.println("DONE");

    Serial.print("Creating new Log File...");
    File logFile = SD.open(fileName, FILE_WRITE);
    Serial.print("...");
    if (logFile) {
        Serial.println("DONE");
        logFile.close();
    } else {
        Serial.println("ERROR");
    }

    Serial.print("Setting pin modes...");
    pinMode(BTN, INPUT);
    Serial.print("...");
    pinMode(INDICATOR, OUTPUT);
    Serial.println("DONE");

    if (setClock) {
        Serial.print("Setting Clock...");
        Time t(2015, 10, 9, 12, 00, 00, Time::kFriday);
        rtc.time(t);
        Serial.println("DONE");
    }

    counter = 0;

    Serial.print("Initializing Display...");
    displayContent[0] = displayContent[1] = "---------------";
    push("IDLE");
    push("PUSH BTN TO STRT");
    Serial.println("DONE");
}

void loop() {
    int toggleRecording = digitalRead(BTN);

    if (toggleRecording && recording) {
        recording = false;
        push("Complete");
    } else if (toggleRecording && !recording) {
        recording = true;
        push("Starting...");
    }

    if (recording) {
        if (fiveInterval) {
            recordReading(counter++);
        }
    }
}

void recordReading(counter) {

}

void push(String text) {
    displayContent[1].toCharArray( displayContent[0], 16 );
    if( sizeof(text)/sizeof(char) < 16 ){
        text = bufferString(text, "*");
    }
    text.toCharArray( displayContent[1], 16 );

    lcd.setCursor(0,0);
    lcd.println(displayContent[0]);
    lcd.setcursor(0,1);
    lcd.println(displayContent[1]);
}

String copyString(arr) {
    int arrLength = sizeof(arr)/sizeof(char);
    char copy[arrLength] = {};

    for(int i = 0; i < arrLength; i++ ) {
        copy[i] = arr[i];
    }
    return copy;
}

String bufferString(text, bufferChar) {

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

