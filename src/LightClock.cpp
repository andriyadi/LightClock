//
// Created by Andri Yadi on 10/13/16.
//

#include "Arduino.h"
#include <Ticker.h>
#include <ws2812_i2s.h>

#if defined(ESP8266)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif
#include <Wire.h>  // must be incuded here so that Arduino library object file references work
#include <RtcDS3231.h>

#include <TM1637Display.h>
// Module connection pins (Digital Pins)
#define CLK 0
#define DIO 10



const uint8_t SEG_DONE[] = {
        SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
        SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
        SEG_C | SEG_E | SEG_G,                           // n
        SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
};

RtcDS3231 Rtc;
TM1637Display display(CLK, DIO);
Ticker timeTicker;

#define NUM_LEDS 16

const uint8_t ONE_PIXEL_FOR_EVERY_SECONDS  = roundf(60*1.0f/NUM_LEDS);

static WS2812 ledstrip;
static Pixel_t pixels[NUM_LEDS];

//static uint16_t theSecond = 0;

#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt)
{
    char datestring[20];

    snprintf_P(datestring,
               countof(datestring),
               PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
               dt.Month(),
               dt.Day(),
               dt.Year(),
               dt.Hour(),
               dt.Minute(),
               dt.Second() );
    Serial.print(datestring);
}

void timeTicked() {

    if (!Rtc.IsDateTimeValid())
    {
        // Common Cuases:
        //    1) the battery on the device is low or even missing and the power line was disconnected
        Serial.println("RTC lost confidence in the DateTime!");
    }

    RtcDateTime now = Rtc.GetDateTime();
    printDateTime(now);
    Serial.println();

    RtcTemperature temp = Rtc.GetTemperature();
    Serial.print(temp.AsFloat());
    Serial.println("C");

//    theSecond++;
//
    uint16_t theMinute, theHour, theSecond;
    uint8_t segto;
//
//    theMinute = (theSecond % 3600) / 60;
//    theHour = (theSecond % 86400) / 3600;

    theSecond = now.Second();
    theMinute = now.Minute();
    theHour = now.Hour();

    //Serial.printf("%d sec\n", theSecond);

    if (theSecond % 5 == 0 && theSecond > 0) {
        display.showNumberDec(temp.AsFloat() * 100, true);
        segto = 0x80 | display.encodeDigit(temp.AsWholeDegrees() % 10);
        display.setSegments(&segto, 1, 1);
        delay(1000);
    }
    else {
        display.showNumberDec(theHour * 100 + theMinute, true);
        delay(500);
        segto = 0x80 | display.encodeDigit(theHour % 10);
        display.setSegments(&segto, 1, 1); //add colon to separate values
        delay(500);
    }

    uint16_t second60 = theSecond % 60;
    uint16_t pixelNo = second60 / ONE_PIXEL_FOR_EVERY_SECONDS;
    Serial.printf("Pixel %d\n", pixelNo);

    for(uint8_t i = 0; i< NUM_LEDS; i++)
    {
        if (i == pixelNo) {
            pixels[i].R = random(40, 255);//50 + (random(SECOND_TO_PIXEL_GAP) * random(12));
            pixels[i].G = random(40, 255);//70 + (random(SECOND_TO_PIXEL_GAP) * random(12));
            pixels[i].B = random(40, 255);//120 + (random(SECOND_TO_PIXEL_GAP) * random(12));
        }
        else {
            pixels[i].R = 0;
            pixels[i].G = 0;
            pixels[i].B = 0;
        }
    }

//    pixels[pixelNo-1].R = 100;
//    pixels[pixelNo-1].G = 100;
//    pixels[pixelNo-1].B = 100;

    ledstrip.show(pixels);
}



void setup() {
    Serial.begin(115200);
    while(!Serial);

    ledstrip.init(NUM_LEDS);

    display.setBrightness(0x0f);
    //timeTicker.attach_ms(1000, timeTicked);

    Serial.print("compiled: ");
    Serial.print(__DATE__);
    Serial.println(__TIME__);

    //--------RTC SETUP ------------
    Rtc.Begin();

    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    printDateTime(compiled);
    Serial.println();

    if (!Rtc.IsDateTimeValid())
    {
        // Common Cuases:
        //    1) first time you ran and the device wasn't running yet
        //    2) the battery on the device is low or even missing

        Serial.println("RTC lost confidence in the DateTime!");

        // following line sets the RTC to the date & time this sketch was compiled
        // it will also reset the valid flag internally unless the Rtc device is
        // having an issue

        Rtc.SetDateTime(compiled);
    }

    if (!Rtc.GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled)
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }
    else if (now > compiled)
    {
        Serial.println("RTC is newer than compile time. (this is expected)");
    }
    else if (now == compiled)
    {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }

    // never assume the Rtc was last configured by you, so
    // just clear them to your needed state
    Rtc.Enable32kHzPin(false);
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
}

void loop() {
    timeTicked();
}