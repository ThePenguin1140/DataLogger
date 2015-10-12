#include <DHT.h> //Temp sensor
#include <SPI.h> // File System
#include <SD.h>  // SD Card 
#include <LiquidCrystal.h>  //Screen
#include <DS1302.h> //Clock

#define RST 4
#define DAT 3
#define CLK 2

boolean setClock = false;

DS1302 rtc(RST, DAT, CLK);

String displayContent[2][16];
int counter;

void setup() {
    Serial.begin(9600);

    if (setClock) {
        Time t(2015, 10, 9, 12, 00, 00, Time::kFriday);
        rtc.time(t);
    }

    counter = 0;
    displayContent[0] = displayContent[1] = "---------------";
    push("IDLE");
    push("PUSH BTN 2 STR");
}

void loop() {
    if (toggleRecording && recording) {
        recording = false;
        push("Complete");
    } else if (toggleRecording && !recording) {
        recording = true;
        push("Starting...");
    }

    if (recording) {
        if (fiveInterval) {
            recordReading(counter);
        }
    }
}

void push(String text) {
    displayContent[0] = copyArr( displayContent[1] );
    if( sizeof(text)/sizeof(char) < 16 ){
        bufferString(text, "*");
    }
    displayContent[1] = text;
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

