static const uint16_t screenWidth  = 320;
static const uint16_t screenHeight = 240;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[2][ screenWidth * 10 ];

#if LV_USE_LOG != 0
/* Serial debugging */
void my_print(const char * buf) {
  Serial.printf(buf);
  Serial.flush();
}
#endif

/* Display flushing */
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p ) {
  M5.Lcd.startWrite();
  M5.Lcd.pushImage(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1, (uint16_t*)color_p);

  lv_disp_flush_ready( disp );
}

void my_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data ) {
  auto t = M5.Touch.getDetail();

  if(!t.isPressed()) {
    data->state = LV_INDEV_STATE_REL;
  } else {
    data->state = LV_INDEV_STATE_PR; 
    /*Set the coordinates*/
    if(!(t.y >= 240)){
      data->point.x = t.x;
      data->point.y = t.y;
    } else {
      data->state = LV_INDEV_STATE_REL;
    }
  }
  //Serial.printf("x = %4d, y = %4d\n", t.x, t.y);
}

void add();
void aftersetup();
void displayupdate();
bool lvwait = false;

void lvtask(void *pvParameters){
  while(1){
    if(!lvwait){
      lv_tick_inc(LV_DISP_DEF_REFR_PERIOD);  /*Force task execution on wake-up*/
      lv_task_handler();                     /*Call `lv_task_handler()` manually to process the wake-up event*/
      //lv_timer_handler(); /* let the GUI do its work */
      displayupdate();
    }
    delay( 5 );
  }
}

void setup() {
#if LV_USE_LOG != 0
  lv_log_register_print_cb( my_print ); /* register print function for debugging */
#endif
  auto cfg = M5.config();
  M5.begin(cfg);

  Serial.begin( 115200 ); /* prepare for possible serial debug */

  String LVGL_Arduino = "Hello Arduino! ";
  LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();


  Serial.println( LVGL_Arduino );
  Serial.println( "I am LVGL_Arduino" );

/* sdupdater */
  SDUCfg.setLabelMenu("< Menu");               // BtnA label: load menu.bin
  SDUCfg.setLabelSkip("Launch");               // BtnB label: skip the lobby countdown and run the app
  SDUCfg.setLabelSave("Save");                 // BtnC label: save the sketch to the SD
  SDUCfg.setAppName("WiFi setting");         // lobby screen label: application name
  SDUCfg.setBinFileName("/WiFi-setting.bin"); // if file path to bin is set for this app, it will be checked at boot and created if not exist

  checkSDUpdater(
    SD,           // filesystem (default=SD)
    MENU_BIN,     // path to binary (default=/menu.bin, empty string=rollback only)
    5000,        // wait delay, (default=0, will be forced to 2000 upon ESP.restart() )
    TFCARD_CS_PIN // usually default=4 but your mileage may vary
  );
/* sdupdater */

  lv_init();

  M5.Lcd.begin();
  M5.Lcd.setSwapBytes(true);

  lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * 10 );

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init( &disp_drv );
  /*Change the following line to your display resolution*/
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register( &disp_drv );

  /*Initialize the (dummy) input device driver*/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init( &indev_drv );
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register( &indev_drv );

  Serial.println( "Setup done" );

  add();

  //xTaskCreatePinnedToCore(lvtask, "lvtask", 4096, NULL, 1, NULL, 1);

  aftersetup();
}
