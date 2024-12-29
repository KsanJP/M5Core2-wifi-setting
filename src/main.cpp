#include <Arduino.h>
#include <M5Unified.h>
#include <lvgl.h>

#include <WiFi.h>

#include <SD.h>
//#define TFCARD_CS_PIN 4
#include <ESP32-targz.h> // optional: https://github.com/tobozo/ESP32-targz
#include <M5StackUpdater.h>

//test

lv_obj_t *tabview, *tab_main, *tab_wifi, *tab_apps, *blabel, *kb, *label_USB, *label_CPU, *label_BAT, *Timelabel;

#include "setup.hpp"
#include "wifis.hpp"

static void ta_event_cb(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * ta = lv_event_get_target(e);
  if(code == LV_EVENT_FOCUSED || code == LV_EVENT_CLICKED) {
    lv_keyboard_set_textarea(kb, ta);
    lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
  }

  if(code == LV_EVENT_DEFOCUSED || code == LV_EVENT_CANCEL) {
    lv_keyboard_set_textarea(kb, NULL);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
  }
};

static void btn_event_cb(lv_event_t * e) {
  lv_obj_t * btn = lv_event_get_target(e);
  lv_obj_t * label = lv_obj_get_child(btn, 0);
  if(WiFi.status() != WL_CONNECTED) {
    lv_label_set_text(label, "plz WiFi");
    return;
  }
  lv_label_set_text(label, "...");
  configTzTime("JST-9", "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");
  //configTime(3600L * 9, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");
  //configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  delay(1600);
  struct tm timeInfo;
  while (!getLocalTime(&timeInfo, 1000)) {
    //delay(1000);
  }
  time_t t = time(nullptr)+1;
  while (t > time(nullptr));
  //M5.Rtc.setDateTime( gmtime( &t ) );
  M5.Rtc.setDateTime( timeInfo );
  Serial.println("NTP set.");

  lv_label_set_text(label, "OK!");
}

int tabs, id = 0;
void add() {
  /*Create a Tab view object*/
  tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 30);

  tab_main = lv_tabview_add_tab(tabview, LV_SYMBOL_HOME "Main");
  tab_wifi = lv_tabview_add_tab(tabview, LV_SYMBOL_WIFI "WiFi");
  tab_apps = lv_tabview_add_tab(tabview, LV_SYMBOL_LIST "apps");
  tabs = 3;

  String LVGL_Arduino = "Hello Arduino! ";
  LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

  {
    /* Hello and バージョン */
    lv_obj_t *label = lv_label_create( tab_main );
    lv_label_set_text( label, LVGL_Arduino.c_str() );
    lv_obj_align( label, LV_ALIGN_TOP_MID, 0, 0 );

    blabel = lv_label_create( tab_main );
    lv_label_set_text( blabel, "-" );
    lv_obj_align( blabel, LV_ALIGN_TOP_MID, 0, 20 );
  }
  {
    // USB
    lv_obj_t *obj = lv_obj_create(tab_main);
    lv_obj_set_size(obj, 75, 40);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_align( obj, LV_ALIGN_LEFT_MID, 0, 0 );
    lv_obj_t *label = lv_label_create(obj);
    lv_label_set_text(label, LV_SYMBOL_USB "USB");
    lv_obj_center(label);
    label_USB = lv_label_create(tab_main);
    lv_label_set_text(label_USB, "---------");
    lv_obj_align_to( label_USB, obj, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
  }
  {
    // CPU
    lv_obj_t *obj = lv_obj_create(tab_main);
    lv_obj_set_size(obj, 75, 40);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_align( obj, LV_ALIGN_CENTER, 0, 0 );
    lv_obj_t *label = lv_label_create(obj);
    lv_label_set_text(label, LV_SYMBOL_SETTINGS "CPU");
    lv_obj_center(label);
    label_CPU = lv_label_create(tab_main);
    lv_label_set_text(label_CPU, "---------");
    lv_obj_align_to( label_CPU, obj, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
  }
  {
    // BAT
    lv_obj_t *obj = lv_obj_create(tab_main);
    lv_obj_set_size(obj, 75, 40);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_align( obj, LV_ALIGN_RIGHT_MID, 0, 0 );
    lv_obj_t *label = lv_label_create(obj);
    lv_label_set_text(label, LV_SYMBOL_BATTERY_FULL "BAT");
    lv_obj_center(label);
    label_BAT = lv_label_create(tab_main);
    lv_label_set_text(label_BAT, "---------");
    lv_obj_align_to( label_BAT, obj, LV_ALIGN_OUT_BOTTOM_MID, 0, 0 );
  }
  {
    // NTP
    lv_obj_t *btn = lv_btn_create(tab_apps);
    lv_obj_set_size(btn, 120, 50);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_align( btn, LV_ALIGN_CENTER, 0, 0 );
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "NTP");
    lv_obj_center(label);
    Timelabel = lv_label_create(tab_main);
    lv_obj_align( Timelabel, LV_ALIGN_TOP_MID, 0, 40 );
    lv_label_set_text(Timelabel, "----/--/-- --:--:--");
  }
  /* キーボード */
  /*Create a keyboard to use it with an of the text areas*/
  kb = lv_keyboard_create(lv_scr_act());
  /*lv_keyboard_set_textarea(kb, ta);*/
  lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);

  /*Create a slider in the center of the display*/
  lv_obj_t * volume_slider = lv_slider_create(tab_apps);
  lv_obj_align(volume_slider, LV_ALIGN_TOP_MID, 0, 20);
  //lv_obj_add_event_cb(volume_slider, volume_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
  lv_obj_add_event_cb( volume_slider, [](lv_event_t * e){
    lv_obj_t * slider = lv_event_get_target(e);
    int vol = (int)lv_slider_get_value(slider) * 2.55;
    Serial.printf("%d\n", vol);
    M5.Speaker.setVolume(vol);
  } , LV_EVENT_VALUE_CHANGED, NULL);

  wifi_add();
}


#define DEV false
void aftersetup() {
  if( DEV ) {
    wifi_connect_check("ssid", "pwd");
  } else {
    wifi_connect_check("", "");
  }
}

void displayupdate();
void loop() {
  wifi_loop();

  lv_tick_inc(LV_DISP_DEF_REFR_PERIOD);  /*Force task execution on wake-up*/
  lv_task_handler();                     /*Call `lv_task_handler()` manually to process the wake-up event*/
  float batVol = M5.Power.Axp192.getBatteryVoltage();
  float batCur = M5.Power.Axp192.getBatteryChargeCurrent();

  float batPercentage = ( batVol < 3.2 ) ? 0 : ( batVol - 3.2 ) * 100;

  char buf[100];
  sprintf(buf, "Baterry: %1.3f,Current: %.1f,%1.1f%%", batVol, batCur, batPercentage);
  lv_label_set_text(blabel, buf);
  sprintf(buf, "%3.1fV\n%4.1fmA", M5.Power.Axp192.getVBUSVoltage(), M5.Power.Axp192.getVBUSCurrent());
  lv_label_set_text(label_USB, buf);
  sprintf(buf, "%3.3fV\n%3.1f°C", M5.Power.Axp192.getAPSVoltage(), M5.Power.Axp192.getInternalTemperature());
  lv_label_set_text(label_CPU, buf);
  sprintf(buf, "%3.3fV\n%4.1fmA\n%1.1f%%", M5.Power.Axp192.getBatteryVoltage(), (M5.Power.Axp192.getBatteryChargeCurrent()>0)?M5.Power.Axp192.getBatteryChargeCurrent():(M5.Power.Axp192.getBatteryDischargeCurrent()*-1), batPercentage);
  lv_label_set_text(label_BAT, buf);

  static constexpr const char* const wd[7] = {"Sun","Mon","Tue","Wed","Thr","Fri","Sat"};
  auto dt = M5.Rtc.getDateTime();
  lv_label_set_text_fmt(Timelabel, "RTC: %04d/%02d/%02d(%s) %02d:%02d:%02d"
               , dt.date.year
               , dt.date.month
               , dt.date.date
               , wd[dt.date.weekDay]
               , dt.time.hours
               , dt.time.minutes
               , dt.time.seconds
               );

  displayupdate();
}

void displayupdate() {
  M5.update();
  if(M5.BtnA.wasClicked()){
    id--;
    if(id < 0) {
      id = tabs-1;
    }
    lv_tabview_set_act(tabview, id, LV_ANIM_OFF);
  } else if(M5.BtnB.wasClicked()){

  } else if(M5.BtnC.wasClicked()){
    id++;
    if(id >= tabs) {
      id %= tabs;
    }
    lv_tabview_set_act(tabview, id, LV_ANIM_OFF);
  }

  delay(5);
}