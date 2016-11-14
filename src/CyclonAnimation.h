//
// Created by Andri Yadi on 8/27/16.
//

#ifndef FBLIKE_CYCLONANIMATION_H
#define FBLIKE_CYCLONANIMATION_H

#include "ESPectro_Neopixel.h"
#include <NeoPixelAnimator.h>

typedef std::function<void()> AnimationCompletedCallback;

class CyclonAnimation {
public:
    CyclonAnimation(ESPectro_Neopixel_DMA &neopixel);
    ~CyclonAnimation();
    void start();
    void end();
    void loop();
    void onAnimationCompleted(AnimationCompletedCallback cb);
    void setAnimationDirection(bool up);
    boolean isAnimating();

private:
    ESPectro_Neopixel_DMA &neopixel_;
    NeoPixelAnimator *animator_ = NULL;

    boolean animationPrevStarted_ = false;

    AnimationCompletedCallback animCompletedCb_;

    uint16_t lastPixel_ = 0; // track the eye position
    int8_t moveDir_ = 1;

    void fadeAll(uint8_t darkenBy);
    void setPixelColor(uint16_t idx);
};


#endif //FBLIKE_CYCLONANIMATION_H
