// scan SSID
#define SSIDLIMIT 30
String ssid_rssi_str[SSIDLIMIT];
String ssid_str[SSIDLIMIT];
String WIFI_Form_str(int ssid_num/* = -1*/){
  /*if(ssid_num == -1) {
    Serial.println("wifi scan start");
    ssid_num = WiFi.scanNetworks(true);
    Serial.println("scan done\r\n");
  }*/

  if (ssid_num == 0) {
    Serial.println("no networks found");
  } else {
    Serial.printf("%d networks found\r\n\r\n", ssid_num);
    if (ssid_num > SSIDLIMIT) ssid_num = SSIDLIMIT;
    for (int i = 0; i < ssid_num; ++i) {
      ssid_str[i] = WiFi.SSID(i);
      String wifi_auth_open = ((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
      ssid_rssi_str[i] = ssid_str[i] + " (" + WiFi.RSSI(i) + "dBm)" + wifi_auth_open;
      //ssid_rssi_str[i] = ssid_str[i] + wifi_auth_open;
      Serial.printf("%d: %s\r\n", i, ssid_rssi_str[i].c_str());
      delay(10);
    }
  }

  String str = "";
  for(int i=0; i<ssid_num; i++){
    str += ssid_rssi_str[i];
    if (i < ssid_num - 1) {
      str += "\n"; // 最後の要素には改行を追加しない
    }
  }

  return str;
}



void wifi_add() {
  lv_obj_set_flex_flow(tab_wifi, LV_FLEX_FLOW_COLUMN);
}

lv_obj_t *wifi_select_dd, *wifi_pass_input;
bool wifichecking = false;
int wificheck_count = 0;
lv_obj_t * tab3_label;

static void ta_event_cb(lv_event_t * e);

void wifi_connect_check(String s_ssid, String s_pwd = "");
static void connect_btn_event_handler(lv_event_t * e){
  char select_ssid[100];
  lv_dropdown_get_selected_str(wifi_select_dd, select_ssid, sizeof(select_ssid));

  auto input_ssid = strtok(select_ssid, " ");

  auto select_pwd = lv_textarea_get_text(wifi_pass_input);
  char input_pwd[100];
  sprintf(input_pwd, "%s", select_pwd);

  Serial.printf("ssid: '%s'\n", input_ssid);
  Serial.println("pwd: '***'\n");
  //Serial.printf("pwd: '%s'\n", input_pwd);

  wifi_connect_check(input_ssid, input_pwd);
}
void wifisetting(String wifis) {
  lvwait = true;
  lv_tick_inc(LV_DISP_DEF_REFR_PERIOD);
  lv_task_handler();

  lv_obj_clean(tab_wifi);
  lv_obj_set_flex_align(tab_wifi, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);


  /* wifi select */
  wifi_select_dd = lv_dropdown_create(tab_wifi);
  lv_dropdown_set_options(wifi_select_dd, wifis.c_str());
  //lv_obj_add_event_cb(wifi_select_dd, dd_event_handler, LV_EVENT_ALL, NULL);
  lv_obj_set_size(wifi_select_dd, LV_PCT(100), LV_SIZE_CONTENT);

  /* password input */
  wifi_pass_input = lv_textarea_create(tab_wifi);
  lv_obj_add_event_cb(wifi_pass_input, ta_event_cb, LV_EVENT_ALL, kb);
  lv_textarea_set_placeholder_text(wifi_pass_input, "Hello");
  lv_textarea_set_text(wifi_pass_input, "");
  lv_textarea_set_one_line(wifi_pass_input, true);
  lv_obj_set_size(wifi_pass_input, LV_PCT(100), LV_SIZE_CONTENT);

  /* send button */
  lv_obj_t * sendbtn = lv_btn_create(tab_wifi);
  lv_obj_add_event_cb(sendbtn, connect_btn_event_handler, LV_EVENT_CLICKED, NULL);
  lv_obj_set_size(sendbtn, LV_PCT(100), LV_SIZE_CONTENT);

  lv_obj_t * sendbtn_label = lv_label_create(sendbtn);
  lv_label_set_text(sendbtn_label, "connect!");
  lv_obj_center(sendbtn_label);


  /* error label */
  lv_obj_t * error_label = lv_label_create(tab_wifi);
  lv_label_set_recolor(error_label, true);
  lv_label_set_text(error_label, "#ff0000 can't connect wifi#");
  lv_obj_center(error_label);

  lvwait = false;
}

static void disconnect_btn_event_handler(lv_event_t * e){
  wifi_connect_check("", "");

  Serial.println("scan start...");
  lv_label_set_text(tab3_label, "scanning...");
  WiFi.disconnect();
  WiFi.scanNetworks(true);

  wifichecking = false;
}

void wifi_connect_check(String s_ssid, String s_pwd){
  lvwait = true;
  lv_tick_inc(LV_DISP_DEF_REFR_PERIOD);
  lv_task_handler();

  lv_obj_clean(tab_wifi);
  lv_obj_set_flex_align(tab_wifi, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);


  /* spinner */
  lv_obj_t * spinner = lv_spinner_create(tab_wifi, 1000, 60);
  lv_obj_set_size(spinner, 100, 100);
  lv_obj_center(spinner);
  /* connecting... */
  tab3_label = lv_label_create(tab_wifi);
  lv_label_set_text(tab3_label, "connecting...");
  lv_obj_align( tab3_label, LV_ALIGN_BOTTOM_MID, 0, 0 );

  lvwait = false;

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  if(s_ssid == "") {
    WiFi.begin();
  } else {
    WiFi.begin(s_ssid, s_pwd);
  }
  wifichecking = true;
  wificheck_count = 0;
}

lv_obj_t * other_data_label;
void wifi_loop() {
  if(wifichecking) {
    wificheck_count++;
    if(WiFi.status() == WL_CONNECTED){
      //wifi connected
      Serial.println();
      Serial.println("connected!");

      wifichecking = false;


      lvwait = true;
      lv_tick_inc(LV_DISP_DEF_REFR_PERIOD);
      lv_task_handler();

      lv_obj_clean(tab_wifi);
      lv_obj_set_flex_align(tab_wifi, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

      char ssidBuffer[32];
      WiFi.SSID().toCharArray(ssidBuffer, 32); // WiFi.SSID()を文字列に変換
      printf("Connected to: %s\n", ssidBuffer);

      /* connected! */
      lv_obj_t * label = lv_label_create(tab_wifi);
      lv_label_set_text(label, "connected!");

      /* SSID! */
      lv_obj_t * ssid_label = lv_label_create(tab_wifi);
      lv_label_set_text_fmt(ssid_label, "ssid:%s", ssidBuffer);

      /* other data */
      other_data_label = lv_label_create(tab_wifi);
      //lv_label_set_text_fmt(other_data_label, "ip:%s, RSSI:%d", WiFi.localIP().toString().c_str(), WiFi.RSSI());

      /* disconnect button */
      lv_obj_t * disconnectbtn = lv_btn_create(tab_wifi);
      lv_obj_add_event_cb(disconnectbtn, disconnect_btn_event_handler, LV_EVENT_CLICKED, NULL);

      lv_obj_t * disconnectbtn_label = lv_label_create(disconnectbtn);
      lv_label_set_text(disconnectbtn_label, "disconnect");
      lv_obj_center(disconnectbtn_label);

      lvwait = false;
    } else if( wificheck_count * 5 > 3000 ){
      //wifi failed
      Serial.println("scan start...");
      lv_label_set_text(tab3_label, "scanning...");
      WiFi.disconnect();
      WiFi.scanNetworks(true);

      wifichecking = false;
    } else if( wificheck_count * 5 % 100 == 0 ){
      Serial.print(".");
    }
  }
  if(WiFi.status() == WL_CONNECTED) {
    lv_label_set_text_fmt(other_data_label, "ip:%s, RSSI:%d", WiFi.localIP().toString().c_str(), WiFi.RSSI());
  }

  if(WiFi.scanComplete() >= 0) {
    wifisetting(WIFI_Form_str(WiFi.scanComplete()));
    WiFi.scanDelete();
  }
}