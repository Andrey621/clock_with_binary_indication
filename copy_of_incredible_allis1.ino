#include <iarduino_RTC.h>                                // Библиотека часов
#include <avr/wdt.h>                                     // Библиотека сна
#include <avr/sleep.h>
#include <avr/eeprom.h>                                  // Библиотека для EEPROM
#include "GyverButton.h"

#define BTN_PIN_HOUR 4                                   // кнопка для часов
#define BTN_PIN_MINUTE 3                                 // кнопка для минут

#define FOUR_SECOND 1<<12
#define ONE_SECOND 1<<10

#define SET_TIME 1                                       // Если нужно установить время при компиляции (берет время из системы)

#if SET_TIME

//  Определяем системное время:                           // Время загрузки скетча.
const char* strM = "JanFebMarAprMayJunJulAugSepOctNovDec"; // Определяем массив всех вариантов текстового представления текущего месяца.
const char* sysT = __TIME__;                              // Получаем время компиляции скетча в формате "SS:MM:HH".
const char* sysD = __DATE__;                              // Получаем дату  компиляции скетча в формате "MMM:DD:YYYY", где МММ - текстовое представление текущего месяца, например: Jul.
//  Парсим полученные значения sysT и sysD в массив i:    // Определяем массив «i» из 6 элементов типа int, содержащий следующие значения: секунды, минуты, часы, день, месяц и год компиляции скетча.
const int i[6] {(sysT[6] - 48) * 10 + (sysT[7] - 48), (sysT[3] - 48) * 10 + (sysT[4] - 48), (sysT[0] - 48) * 10 + (sysT[1] - 48), (sysD[4] - 48) * 10 + (sysD[5] - 48), ((int)memmem(strM, 36, sysD, 3) + 3 - (int)&strM[0]) / 3, (sysD[9] - 48) * 10 + (sysD[10] - 48)};

#endif

const int leds_hour[] = {12, 11, 10, 9};
const int leds_minute[] = {8, 7, 6, 14, 15, 16};
const size_t num_leds_hour = sizeof(leds_hour) / sizeof(leds_hour[0]);
const size_t num_leds_minute = sizeof(leds_minute) / sizeof(leds_minute[0]);

byte hour, minute;

iarduino_RTC watch(RTC_DS3231);
GButton hour_button(BTN_PIN_HOUR);
GButton min_button(BTN_PIN_MINUTE);

void setup() {

  watch.begin();                                          // Инициируем RTC модуль
  Serial.begin(9600);

#if SET_TIME
  if(!eeprom_read_byte(0)){
  watch.settime(i[0], i[1], i[2], i[3], i[4], i[5]);    // Устанавливаем время в модуль: i[0] сек, i[1] мин, i[2] час, i[3] день, i[4] месяц, i[5] год, без указания дня недели.
  eeprom_write_byte(0, 1);
 }
#endif

  for (size_t i = 0; i < num_leds_hour; i++)
    pinMode(leds_hour[i], OUTPUT);

  for (size_t i = 0; i < num_leds_minute; i++)
    pinMode(leds_minute[i], OUTPUT);

}

void loop() {

  hour_button.tick();
  min_button.tick();

  Serial.println(FOUR_SECOND);

  if (min_button.isStep()) {                                                                  // удержание
    long long time_now = millis();
    bool is_button_presed = false;
    minute = 0;
    while (millis() - time_now < FOUR_SECOND) {
      if (is_button_presed) {
        time_now = millis();
        while (millis() - time_now < FOUR_SECOND) {
          min_button.tick();
          if (min_button.isPress() || min_button.isStep()) {
            if (minute > 60)
              minute = 0;
            minute++;
            time_now = millis();
          }
          for (size_t i = 0; i < num_leds_minute; i++)
            digitalWrite(leds_minute[num_leds_minute - i - 1], bitRead(minute, i));
        }
          for (size_t i = 0; i < num_leds_minute; i++)
            digitalWrite(leds_minute[i], LOW);
          delay(100);
          for (size_t i = 0; i < num_leds_minute; i++)
            digitalWrite(leds_minute[i], HIGH);
          delay(100);
          for (size_t i = 0; i < num_leds_minute; i++)
            digitalWrite(leds_minute[i], LOW);
          watch.settime(0, minute);
      }
      for (size_t i = 0; i < num_leds_minute && !is_button_presed; i++) {
        min_button.tick();
        if (min_button.isSingle())
          is_button_presed = true;
        digitalWrite(leds_minute[i], HIGH);
        delay(50);
      }
      for (size_t i = 0; i < num_leds_minute && !is_button_presed; i++) {
        min_button.tick();
        if (min_button.isSingle())
          is_button_presed = true;
        digitalWrite(leds_minute[i], LOW);
        delay(50);
      }
    }
  }

  if (hour_button.isStep()) {                                                                 // удержание
    long long time_now = millis();
    bool is_button_presed = false;
    hour = 0;
    while (millis() - time_now < FOUR_SECOND) {
      if (is_button_presed) {
        time_now = millis();
        while (millis() - time_now < FOUR_SECOND) {
          hour_button.tick();
          if (hour_button.isPress() || hour_button.isStep()) {
            if (hour > 12)
              hour = 0;
            hour++;
            time_now = millis();
          }
          for (size_t i = 0; i < num_leds_hour; i++)
            digitalWrite(leds_hour[num_leds_hour - i - 1], bitRead(hour, i));
        }
          for (size_t i = 0; i < num_leds_hour; i++)
            digitalWrite(leds_hour[i], LOW);
          delay(100);
          for (size_t i = 0; i < num_leds_hour; i++)
            digitalWrite(leds_hour[i], HIGH);
          delay(100);
          for (size_t i = 0; i < num_leds_hour; i++)
            digitalWrite(leds_minute[i], LOW);
          watch.settime(0, watch.gettime("i"), hour);
      }
      for (size_t i = 0; i < num_leds_hour && !is_button_presed; i++) {
        hour_button.tick();
        if (hour_button.isSingle())
          is_button_presed = true;
        digitalWrite(leds_hour[i], HIGH);
        delay(50);
      }
      for (size_t i = 0; i < num_leds_hour && !is_button_presed; i++) {
        hour_button.tick();
        if (hour_button.isSingle())
          is_button_presed = true;
        digitalWrite(leds_hour[i], LOW);
        delay(50);
      }
    }
  }
  
//  if (minute != String(watch.gettime("i")).toInt()) {

    hour = String(watch.gettime("h")).toInt();
    minute = String(watch.gettime("i")).toInt();

    for (size_t i = 0; i < num_leds_hour; i++)
      digitalWrite(leds_hour[num_leds_hour - i - 1], bitRead(hour, i));                      // Установка пинов по времени для часов

    for (size_t i = 0; i < num_leds_minute; i++)
      digitalWrite(leds_minute[num_leds_minute - i - 1], bitRead(minute, i));                // Установка пинов по времени для минут
//  }

  //  wdt_enable(WDTO_1S);                                     // Сон 1 секунду
  //  WDTCSR |= (1 << WDIE);
  //  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  //  sleep_mode();

}

ISR (WDT_vect) {
  wdt_disable();
}
