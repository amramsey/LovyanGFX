/*----------------------------------------------------------------------------/
  Lovyan GFX - Graphics library for embedded devices.

Original Source:
 https://github.com/lovyan03/LovyanGFX/

Licence:
 [FreeBSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)

Author:
 [lovyan03](https://twitter.com/lovyan03)

Contributors:
 [ciniml](https://github.com/ciniml)
 [mongonta0716](https://github.com/mongonta0716)
 [tobozo](https://github.com/tobozo)
/----------------------------------------------------------------------------*/
#if defined (ESP32) || defined (CONFIG_IDF_TARGET_ESP32) || defined (CONFIG_IDF_TARGET_ESP32S2) || defined (ESP_PLATFORM)

#include "Light_PWM.hpp"

#if defined ARDUINO
 #include <esp32-hal-ledc.h>
#else
 #include <driver/ledc.h>
#endif

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  void Light_PWM::init(void)
  {
    std::uint8_t duty = _brightness;
    if (_cfg.invert) duty = ~duty;

#ifdef ARDUINO

    ledcSetup(_cfg.pwm_channel, _cfg.freq, 8);
    ledcAttachPin(_cfg.pin_bl, _cfg.pwm_channel);
    ledcWrite(_cfg.pwm_channel, duty);

#else

    static ledc_channel_config_t ledc_channel;
    {
     ledc_channel.gpio_num   = (gpio_num_t)_cfg.pin_bl;
#if SOC_LEDC_SUPPORT_HS_MODE
     ledc_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
#else
     ledc_channel.speed_mode = LEDC_LOW_SPEED_MODE;
#endif
     ledc_channel.channel    = (ledc_channel_t)_cfg.pwm_channel;
     ledc_channel.intr_type  = LEDC_INTR_DISABLE;
     ledc_channel.timer_sel  = (ledc_timer_t)((_cfg.pwm_channel >> 1) & 3);
     ledc_channel.duty       = duty; // duty;
     ledc_channel.hpoint     = 0;
    };
    ledc_channel_config(&ledc_channel);
    static ledc_timer_config_t ledc_timer;
    {
#if SOC_LEDC_SUPPORT_HS_MODE
      ledc_timer.speed_mode = LEDC_HIGH_SPEED_MODE;     // timer mode
#else
      ledc_timer.speed_mode = LEDC_LOW_SPEED_MODE;
#endif
      ledc_timer.duty_resolution = (ledc_timer_bit_t)8; // resolution of PWM duty
      ledc_timer.freq_hz = _cfg.freq;                        // frequency of PWM signal
      ledc_timer.timer_num = ledc_channel.timer_sel;    // timer index
    };
    ledc_timer_config(&ledc_timer);

#endif

  }

  void Light_PWM::setBrightness(std::uint8_t brightness)
  {
    _brightness = brightness;

    if (_sleep) brightness = 0;
    if (_cfg.invert) brightness = ~brightness;

#ifdef ARDUINO
    ledcWrite(_cfg.pwm_channel, brightness);
#elif SOC_LEDC_SUPPORT_HS_MODE
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, (ledc_channel_t)_cfg.pwm_channel, brightness);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, (ledc_channel_t)_cfg.pwm_channel);
#else
    ledc_set_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)_cfg.pwm_channel, brightness);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)_cfg.pwm_channel);
#endif
  }


//----------------------------------------------------------------------------
 }
}

#endif