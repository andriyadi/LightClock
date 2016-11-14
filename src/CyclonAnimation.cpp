//
// Created by Andri Yadi on 8/27/16.
//

#include "CyclonAnimation.h"

CyclonAnimation::CyclonAnimation(ESPectro_Neopixel_DMA &neopixel):neopixel_(neopixel) {

}

CyclonAnimation::~CyclonAnimation() {
    if (animator_ != NULL) {
        delete animator_;
        animator_ = NULL;
    }
}

const RgbColor CylonEyeColor(HtmlColor(0x7f0000));

void CyclonAnimation::fadeAll(uint8_t darkenBy)
{
    RgbColor color;
    for (uint16_t indexPixel = 0; indexPixel < neopixel_.PixelCount(); indexPixel++)
    {
        color = neopixel_.GetPixelColor(indexPixel);
        color.Darken(darkenBy);
        neopixel_.SetPixelColor(indexPixel, color);
    }
}

void CyclonAnimation::start() {

//    animationPrevStarted_ = true;

    if (animator_ == NULL) {
        animator_ = new NeoPixelAnimator(2);
    }
    else {
        animationPrevStarted_ = true;
        animator_->RestartAnimation(0);
        animator_->RestartAnimation(1);
        return;
    }

//    animationPrevStarted_ = true;

    AnimEaseFunction moveEase =
//      NeoEase::Linear;
//      NeoEase::QuadraticInOut;
//      NeoEase::CubicInOut;
//      NeoEase::QuarticInOut;
//      NeoEase::QuinticInOut;
//      NeoEase::SinusoidalInOut;
      NeoEase::ExponentialInOut;
//      NeoEase::CircularInOut;

    uint16_t PixelCount = neopixel_.PixelCount();
    CyclonAnimation *mySelf = this;

    AnimUpdateCallback moveAnimUpdate = [=](const AnimationParam& param)
    {
        // apply the movement animation curve
        float progress = moveEase(param.progress);

        // use the curved progress to calculate the pixel to effect
        uint16_t nextPixel;
        if (mySelf->moveDir_ > 0)
        {
            nextPixel = progress * PixelCount;
        }
        else
        {
            nextPixel = (1.0f - progress) * PixelCount;
        }

        // if progress moves fast enough, we may move more than
        // one pixel, so we update all between the calculated and
        // the last
        if (mySelf->lastPixel_ != nextPixel)
        {
            for (uint16_t i = mySelf->lastPixel_ + mySelf->moveDir_; i != nextPixel; i += mySelf->moveDir_)
            {
                mySelf->setPixelColor(i);
            }
        }
        mySelf->setPixelColor(nextPixel);

        mySelf->lastPixel_ = nextPixel;

        if (param.state == AnimationState_Completed)
        {
            // reverse direction of movement
            //mySelf->moveDir_ *= -1;

            // done, time to restart this position tracking animation/timer
            //mySelf->animator_->RestartAnimation(param.index);

            mySelf->animator_->StopAnimation(0);
            mySelf->animator_->StopAnimation(1);
        }
    };



    AnimUpdateCallback fadeAnimUpdate = [=](const AnimationParam& param)
    {
        if (param.state == AnimationState_Completed)
        {
            mySelf->fadeAll(10);
            mySelf->animator_->RestartAnimation(param.index);
        }
    };

    animator_->StartAnimation(0, 5, fadeAnimUpdate);

    // take several seconds to move eye fron one side to the other
    animator_->StartAnimation(1, 2000, moveAnimUpdate);

    animationPrevStarted_ = true;
}

void CyclonAnimation::end() {

    animationPrevStarted_ = false;
    animator_->StopAnimation(1);
    animator_->StopAnimation(0);

}

void CyclonAnimation::loop() {

    if (animator_ != NULL) {
        if (animator_->IsAnimating() && animationPrevStarted_) {
            animator_->UpdateAnimations();
            neopixel_.Show();
        }
        else {

//            if (animationPrevStarted_) {
////                Serial.print("X");
////                animationPrevStarted_ = false;
//                Serial.println("YUHU!!!!");
//                end();
//                if (animCompletedCb_) {
//                    animCompletedCb_();
//                }
//            }
        }

//        if (animationPrevStarted_ && !animator_->IsAnimationActive(1)) {
//            Serial.println("YUHU!!!!");
//            end();
//            if (animCompletedCb_) {
//                animCompletedCb_();
//            }
//        }
    }
}

void CyclonAnimation::setPixelColor(uint16_t idx) {
    //neopixel_.SetPixelColor(idx, CylonEyeColor);

    float hue = 360.0f - ((idx*1.0f/neopixel_.PixelCount())*270);

    HslColor color = HslColor(hue/360.0f, 1.0f, 0.5f);
    neopixel_.SetPixelColor(idx, color);
}

void CyclonAnimation::onAnimationCompleted(AnimationCompletedCallback cb) {
    animCompletedCb_ = cb;
}

void CyclonAnimation::setAnimationDirection(bool up) {
    if (up) {
        moveDir_ = 1;
    }
    else {
        moveDir_ = -1;
    }
}

boolean CyclonAnimation::isAnimating() {
    return animationPrevStarted_;// && animator_->IsAnimationActive(1);
}
