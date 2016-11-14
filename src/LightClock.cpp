//
// Created by Andri Yadi on 10/13/16.
//

#include "Arduino.h"
//#include <Ticker.h>
#include <ws2812_i2s.h>

#include <ESP8266WiFiMulti.h>
#include "ESPectro.h"
#include "MakestroCloudClient.h"
#include "DCX_AppSetting.h"
//#include "ESPectro_Neopixel.h"
//#include "CyclonAnimation.h"

#if defined(ESP8266)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif
#include <Wire.h>  // must be incuded here so that Arduino library object file references work
#include <RtcDS3231.h>
#include <TM1637Display.h>

ESPectro esPectro;
ESPectro_Button button;
MakestroCloudClient ioTHubClient("alwin", "93kt97biEKyc3C45xj72TMdTQfoAx9i8vOlMelpCKug4QyqSCvOIfJcrryQUljnh", "dycodexlikecounter");
ESP8266WiFiMulti wifiMulti;
uint16_t initialLike = 0;
volatile int16_t newLike = 0;
bool firstAnimRun = true;
volatile bool shouldAnimate = false;
volatile bool animationDirUp = true;

#define PIXEL_OFFSET    0
//ESPectro_Neopixel_DMA neopixel(16, 5);
//CyclonAnimation cyclonAnimation(neopixel);

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
//Ticker timeTicker;

#define NUM_LEDS_FOR_SECONDS    20   // <==== CHANGE THIS BASED ON AVAILABLE NEOPIXEL LEDS

#define NUM_LEDS_FOR_HOURS       12   // <==== CHANGE THIS BASED ON AVAILABLE NEOPIXEL LEDS

const uint16_t TOTAL_LED_NUM = (NUM_LEDS_FOR_HOURS + NUM_LEDS_FOR_SECONDS);
const uint8_t ONE_PIXEL_FOR_EVERY_SECONDS  = roundf(60 * 1.0f / NUM_LEDS_FOR_SECONDS);
const uint8_t ONE_PIXEL_FOR_EVERY_HOURS  = roundf(60 * 1.0f / NUM_LEDS_FOR_HOURS);

static WS2812 ledstrip;
static Pixel_t pixels[TOTAL_LED_NUM];

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

    if (theSecond % 6 == 0 && theSecond != 0) {
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


    theHour = (theHour > 12)? theHour - 12: theHour;
    for(uint8_t i = 0; i < NUM_LEDS_FOR_HOURS; i++)
    {
        if (i == (theHour - 1)) {
//            pixels[i].R = 0.9*random(40, 255);//50 + (random(SECOND_TO_PIXEL_GAP) * random(12));
//            pixels[i].G = 0.9*random(40, 255);//70 + (random(SECOND_TO_PIXEL_GAP) * random(12));
//            pixels[i].B = 0.9*random(40, 255);//120 + (random(SECOND_TO_PIXEL_GAP) * random(12));
            pixels[i].R = 0;
            pixels[i].G = 100;
            pixels[i].B = 250;
        }
        else {
            pixels[i].R = 0;
            pixels[i].G = 0;
            pixels[i].B = 0;
        }
    }

    uint16_t second60 = theSecond % 60;
    uint16_t pixelNoForSecs = second60 / ONE_PIXEL_FOR_EVERY_SECONDS;
    Serial.printf("Pixel %d\n", pixelNoForSecs);

    for(uint8_t i = NUM_LEDS_FOR_HOURS; i < (NUM_LEDS_FOR_HOURS + NUM_LEDS_FOR_SECONDS); i++)
    {
        //Serial.printf("Neo number %d\n", i);
        if (i == (pixelNoForSecs + NUM_LEDS_FOR_HOURS)) {
            pixels[i].R = 0.9*random(40, 255);//50 + (random(SECOND_TO_PIXEL_GAP) * random(12));
            pixels[i].G = 0.9*random(40, 255);//70 + (random(SECOND_TO_PIXEL_GAP) * random(12));
            pixels[i].B = 0.9*random(40, 255);//120 + (random(SECOND_TO_PIXEL_GAP) * random(12));
        }
        else {
            pixels[i].R = 0;
            pixels[i].G = 0;
            pixels[i].B = 0;
        }
    }

    ledstrip.show(pixels);
}

/*
void setBargraphTo(int16_t likeCount) {
    Serial.printf("Like count: %d\r\n", newLike);

    int totalNeo = neopixel.PixelCount();

    for(int i = 0; i < PIXEL_OFFSET; i++) {
        float hue = 360.0f - ((i*1.0f/totalNeo)*270);

        HslColor color = HslColor(hue/360.0f, 1.0f, 0.5f);
        neopixel.SetPixelColor(i, color);
    }

    int idx;

    for(idx = PIXEL_OFFSET; idx < PIXEL_OFFSET + likeCount; idx++) {

        float hue = 360.0f - ((idx*1.0f/totalNeo)*270);

        HslColor color = HslColor(hue/360.0f, 1.0f, 0.5f);
        neopixel.SetPixelColor(idx, color);
    }

    for(int j = idx; j < neopixel.PixelCount(); j++) {
        neopixel.SetPixelColor(j, HtmlColor(0x000000));
    }

    neopixel.Show();
}
*/

void onMqttConnect() {

    Serial.println("** Connected to the broker **");

    MakestroCloudSubscribedPropertyCallback propsCallback = [=](const String prop, const String value) {
//        Serial.print("incoming: ");
//        Serial.print(prop);
//        Serial.print(" = ");
//        Serial.print(value);
//        Serial.println();

        if (prop.equals("today")) {
            newLike = value.toInt();
            if (newLike < 0) {
                newLike = 0;
            }

//            Serial.println("Saving...");
            AppSetting.anIntVal = newLike;
            AppSetting.save();
//            Serial.println("Saved");
//            Serial.printf("New Like: %d\r\n", newLike);
//            setBargraphTo(newLike);
        }

        if (prop.equals("event")) {
            if (value.equals("like")) {
                //neopixel.startPulsingAnimation(HtmlColor(0x00ffff));

//                cyclonAnimation.end();
//                cyclonAnimation.setAnimationDirection(true);
//                cyclonAnimation.start();

                shouldAnimate = true;
                animationDirUp = true;
            }
            else if (value.equals("unlike")) {
                //neopixel.startPulsingAnimation(HtmlColor(0xff0000));


//                cyclonAnimation.end();
//                cyclonAnimation.setAnimationDirection(false);
//                cyclonAnimation.start();

                shouldAnimate = true;
                animationDirUp = false;
            }
        }

//        if (prop.equals("total_likes")) {
//            if (initialLike == 0) {
//                initialLike = value.toInt();
//            }
//            newLike = value.toInt() - initialLike;
//            if (newLike < 0) {
//                newLike = 0;
//            }
//
//            Serial.printf("New Like: %d\r\n", newLike);
//            setBargraphTo(newLike);
//        }
    };

    ioTHubClient.subscribePropertyWithTopic("data", "event", propsCallback);
    ioTHubClient.subscribePropertyWithTopic("data", "today", propsCallback);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
    Serial.println("** Disconnected from the broker **");
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
    Serial.println("** Subscribe acknowledged **");
}

uint32_t Wheel(byte WheelPos) {
    uint8_t r, g, b;
    WheelPos = 255 - WheelPos;
    if(WheelPos < 85) {
        r = 255 - WheelPos * 3;
        g = 0;
        b = WheelPos * 3;
    }
    else if(WheelPos < 170) {
        WheelPos -= 85;

        g = 255 - WheelPos * 3;
        r = 0;
        b = WheelPos * 3;
    }
    else {
        WheelPos -= 170;
        r = 255 - WheelPos * 3;
        b = 0;
        g = WheelPos * 3;
    }

    return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

void colorWipe(uint8_t wait, uint32_t c = 0, bool upDir = true) {

    for(uint16_t i = 0; i < TOTAL_LED_NUM; i++) {

        uint16_t n = upDir? i: TOTAL_LED_NUM - i - 1;

        pixels[n].R = 0;
        pixels[n].G = 0;
        pixels[n].B = 0;
    }

    for(uint16_t i = 0; i < TOTAL_LED_NUM; i++) {

        uint16_t n = upDir? i: TOTAL_LED_NUM - i - 1;
        uint32_t cl = c == 0? Wheel( ((i * 256 / TOTAL_LED_NUM)) % 255): c;

        pixels[n].R = cl >> 16;
        pixels[n].G = cl >> 8;
        pixels[n].B = cl;

        ledstrip.show(pixels);
        delay(wait);
    }

    delay(200);
    for(uint16_t i = 0; i < TOTAL_LED_NUM; i++) {

        uint16_t n = upDir? i: TOTAL_LED_NUM - i - 1;
        pixels[n].R = 0;
        pixels[n].G = 0;
        pixels[n].B = 0;

        ledstrip.show(pixels);
        delay(wait);
    }
}

void setup() {
    Serial.begin(115200);
    while(!Serial);

    ledstrip.init(NUM_LEDS_FOR_SECONDS + NUM_LEDS_FOR_HOURS);

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


    //FB LIKE

    AppSetting.load();
    AppSetting.debugPrintTo(Serial);


    esPectro.turnOffLED();

    wifiMulti.addAP("TP-LINK_POCKET_3020_F920C2", "53113481");
    wifiMulti.addAP("Sanctuary", "Sanctuary1209");
    wifiMulti.addAP("Andromax-M3Y-C634", "p@ssw0rd");
    wifiMulti.addAP("Andri's iPhone 6s", "11223344");
    wifiMulti.addAP("dycodex", "11223344");

    Serial.println("Connecting Wifi...");
    uint8_t connTrial = 0;
    while (wifiMulti.run() != WL_CONNECTED && connTrial < (500*2*30)) {
        Serial.print(".");
        esPectro.toggleLED();
        delay(500);
        connTrial++;
    }

    esPectro.turnOffLED();

    if (wifiMulti.run() == WL_CONNECTED) {
        Serial.println("");
        Serial.print(F("WiFi connected to: "));
        Serial.println(WiFi.SSID());
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
    }

    ioTHubClient.setClientId("FBLike_Awesome_01");
    ioTHubClient.onConnect(onMqttConnect);
    ioTHubClient.onDisconnect(onMqttDisconnect);
    ioTHubClient.onSubscribe(onMqttSubscribe);
    //ioTHubClient.onMessage(onMqttMessage);

    ioTHubClient.connect();

//    neopixel.Begin();
//    neopixel.Show();

    newLike = AppSetting.anIntVal;
//    setBargraphTo(8);

//    cyclonAnimation.onAnimationCompleted([]() {
//
//        if (firstAnimRun) {
//            firstAnimRun = false;
////            cyclonAnimation.setAnimationDirection(false);
////            cyclonAnimation.start();
//
//            shouldAnimate = true;
//            animationDirUp = false;
//
//        }
//        else {
//
//            Serial.printf("New Like: %d\r\n", newLike);
//            setBargraphTo(newLike);
//        }
//    });

//    cyclonAnimation.start();
//    shouldAnimate = true;

    display.showNumberDec(newLike, true);
    delay(800);

    colorWipe(40, 0, animationDirUp);
}

void loop() {
    if(wifiMulti.run() != WL_CONNECTED) {
        Serial.println("WiFi not connected!");
        delay(100);
        return;
    }

//    if (shouldAnimate) {
//        shouldAnimate = false;
//
//        cyclonAnimation.end();
//        cyclonAnimation.setAnimationDirection(animationDirUp);
//        cyclonAnimation.start();
//    }

//    cyclonAnimation.loop();
//
//    if (!cyclonAnimation.isAnimating()) {
//        //timeTicked();
//    }

    if (shouldAnimate) {
        shouldAnimate = false;

        colorWipe(40, 0, animationDirUp);
    }
    else {
        timeTicked();
    }
}