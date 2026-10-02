#pragma once

#include <Arduino.h>
#include <driver/gpio.h>
#include <initializer_list>

// Hält die Steuerleitungen zum Display im Deep Sleep auf ihrem letzten Pegel.
// Ohne Hold sind die Pins im Deep Sleep hochohmig: offene CMOS-Eingänge am Display
// können Querstrom ziehen, und ein schwebender RST weckt den Controller aus dem Hibernate.
// Die Strapping-Pins 0, 2 und 12 werden nie gehalten (Boot-Modus, Flash-Spannung).
namespace ePaperPinHold {

inline bool isHoldable(uint8_t pin) {
    return pin != 0 && pin != 2 && pin != 12 && GPIO_IS_VALID_OUTPUT_GPIO(pin);
}

// Direkt vor dem Deep Sleep aufrufen
inline void hold(std::initializer_list<uint8_t> pins) {
    for (uint8_t pin : pins) {
        if (isHoldable(pin)) gpio_hold_en((gpio_num_t)pin);
    }
    gpio_deep_sleep_hold_en();
}

// Nach dem Aufwachen vor dem ersten Zugriff auf die Pins aufrufen (vor display.init()),
// sonst bleibt z.B. RST gesperrt und der Controller wird nicht zurückgesetzt
inline void release(std::initializer_list<uint8_t> pins) {
    gpio_deep_sleep_hold_dis();
    for (uint8_t pin : pins) {
        if (isHoldable(pin)) gpio_hold_dis((gpio_num_t)pin);
    }
}

}
