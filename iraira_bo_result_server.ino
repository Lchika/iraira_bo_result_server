#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include <Ticker.h>
#include <map>                  // map型を使用するため
#include "SimpleWebServer.h"
#include "SegDisp.h"
#include "RankingManager.hpp"
#include <Preferences.h>        // 不揮発領域管理用ライブラリ
#include <HTTPClient.h>

//  define定義
#define ONE_DIGIT_PIN       27
#define TWO_DIGIT_PIN       26
#define THREE_DIGIT_PIN     25
#define PIN_HSERIAL_RX      21
#define PIN_HSERIAL_TX      22
#define MAX_TRY_CONNECT       50      // 最大WiFi接続試行回数
#define MAX_TRY_CONNECT_INI   10      // 最大WiFi接続試行回数(初期接続)

//  グローバル変数定義
int output_pins[] = {12, 13, 14, 15, 16, 17, 18, 19};
Ticker ticker1;
//Ticker ticker2;
SegDisp segDisp = SegDisp(output_pins);
bool is_ready_flash = false;
HardwareSerial serial_voice_module(1);
RankingManager *rankingManager;
IPAddress ip(192, 168, 100, 111);        // for fixed IP Address
IPAddress gateway(192, 168, 1, 1);       //
IPAddress subnet(255, 255, 255, 0);      //
// webサーバの生成
SimpleWebServer server("ESP32AP", "12345678", ip, IPAddress(255, 255, 255, 0), 80);
uint8_t root_html[16384];     //  /index.html
uint8_t success_html[16384];  //  /success.html
uint8_t access_html[16384];   //  /access.html
// 不揮発領域アクセス用クラス
Preferences preferences;

//  内部関数定義
static void set_server(void);
static void handle_root_get(void);
static void handle_root_post(String query);
static void analyze_query(String query, std::map<String, String> &map_query);
static void flash_7seg(void);
static String getStringAddedTag(String input, String tag);
static String getStringTime(int time);
static String getHtmlByRank(int rank);
static void make_root_html(void);

/*-------------------------------------------------*/
// Arduinoセットアップ処理
/*-------------------------------------------------*/
void setup(){
  //  シリアルポート初期化
  Serial.begin(9600);
  //  ピンモード初期化
  pinMode(ONE_DIGIT_PIN, OUTPUT);
  pinMode(TWO_DIGIT_PIN, OUTPUT);
  pinMode(THREE_DIGIT_PIN, OUTPUT);

  digitalWrite(ONE_DIGIT_PIN, HIGH);
  digitalWrite(TWO_DIGIT_PIN, HIGH);
  digitalWrite(THREE_DIGIT_PIN, HIGH);
  
  // 不揮発性領域へのアクセス開始
  preferences.begin("ssid_1", false);
  preferences.begin("pass_1", false);
  preferences.begin("ssid_2", false);
  preferences.begin("pass_2", false);
  preferences.begin("num_saved_id", false);

  serial_voice_module.begin(9600, SERIAL_8N1, PIN_HSERIAL_RX, PIN_HSERIAL_TX);
  volume(0x10);

  initParam init_param;
  init_param.call_backs["onDataAdd"] = [](rankingData_S rank_data[], int length){
    play(0x01);
    };
  rankingManager = new RankingManager(init_param);

  segDisp.setDispNumber(0);
  ticker1.attach_ms(1, flash_7seg);
  //ticker2.attach_ms(10000, make_root_html);
  
  //  ランキング表示用HTMLを初期化
  make_root_html();

  //  webサーバ設定処理
  set_server();

  // 一度記憶情報を用いてWiFi接続を試みる
  if (!WiFi.config(ip, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }
  Serial.print("num_saved_id = ");
  Serial.println(preferences.getInt("num_saved_id", 0));
  Serial.print("ssid_1 = ");
  Serial.print(preferences.getString("ssid_1", ""));
  Serial.print(", pass1 = ");
  Serial.println(preferences.getString("pass_1", ""));
  WiFi.begin(preferences.getString("ssid_1", "").c_str(), preferences.getString("pass_1", "").c_str());

  int connect_try_count = 0;
  while (WiFi.status() != WL_CONNECTED && connect_try_count < MAX_TRY_CONNECT_INI) {
    delay(500);
    Serial.print(".");
    connect_try_count++;
  }

  if (WiFi.status() == WL_CONNECTED) {  
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    return;
  } else {
    Serial.println("WiFi connect process time out.");
  }

  // 繰り返し、今度はssid_2を使う
  // ! ToDo 関数化する
  connect_try_count = 0;
  Serial.print("ssid_2 = ");
  Serial.print(preferences.getString("ssid_2", ""));
  Serial.print(", pass2 = ");
  Serial.println(preferences.getString("pass_2", ""));
  WiFi.begin(preferences.getString("ssid_2", "").c_str(), preferences.getString("pass_2", "").c_str());

  while (WiFi.status() != WL_CONNECTED && connect_try_count < MAX_TRY_CONNECT_INI) {
    delay(500);
    Serial.print(".");
    connect_try_count++;
  }

  if (WiFi.status() == WL_CONNECTED) {  
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    return;
  } else {
    Serial.println("WiFi connect process time out.");
  }
  return;
}

/*-------------------------------------------------*/
// Arduinoループ処理
/*-------------------------------------------------*/
void loop(){
  //  esp32へのリクエストを処理する
  server.handle_request();
  //uint32_t time = millis();
  //segDisp.setDispNumber(time / 1000);
  //Serial.println(time / 1000);
  return;
}

/*-------------------------------------------------*/
// webサーバ設定処理
/*-------------------------------------------------*/
static void set_server(){
  // パス設定
  const char *access_path = "/access.html";
  const char *success_path = "/success.html";
  // htmlファイル読み込み用変数
  File html_file;
  size_t size;
  
  // esp32に記憶させてあるhtmlファイルを読み込んでいく
  // access.htmlを読み込む
  SPIFFS.begin();
  // access.htmlを読み込む
  html_file = SPIFFS.open(access_path, "r");
  size = html_file.size();
  html_file.read(access_html, size);
  html_file.close();
  // success.htmlを読み込む
  html_file = SPIFFS.open(success_path, "r");
  size = html_file.size();
  html_file.read(success_html, size);
  html_file.close();
  
  //  urlハンドラの設定
  server.add_handler("/", handle_root_get);
  server.add_handler_post("/", handle_root_post);
  server.add_handler("/access.html", handle_access_get);
  server.add_handler_post("/access.html", handle_access_post);
  server.begin();
  return;
}

/*-------------------------------------------------*/
// GET/ 処理
/*-------------------------------------------------*/
static void handle_root_get(){
  server.send_html(200, (char *)root_html);
  return;
}

/*-------------------------------------------------*/
// POST/ 処理
/*-------------------------------------------------*/
static void handle_root_post(String query){
  static int player_sum = 0;
  std::map<String, String> map_query;

  //  クエリの解析
  analyze_query(query, map_query);
  Serial.println("time = " + map_query["time"] + ", miss = " + map_query["miss"]);

  //  ランキングに登録
  rankingManager->setData(map_query["time"].toInt(), map_query["miss"].toInt());

  //  ランキング表示用HTMLを更新
  make_root_html();

  //  7セグ表示値を更新
  segDisp.setDispNumber(++player_sum);

  server.send_status(200);
  return;
}

// GET /access.html
void handle_access_get(){
  server.send_html(200, (char *)access_html);
}

// POST /access.html
void handle_access_post(String query){
  std::map<String, String> map_query;
  int connect_try_count = 0;

  // クエリの解析
  analyze_query(query, map_query);
  Serial.print("ssid = ");
  Serial.print(map_query["ssid"]);
  Serial.print(", password = ");
  Serial.println(map_query["password"]);

  // WiFi接続処理  
  Serial.println();
  Serial.println();
  if (!WiFi.config(ip, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }
  Serial.print("Connecting to ");
  Serial.println(map_query["ssid"]);
  WiFi.begin(map_query["ssid"].c_str(), map_query["password"].c_str());

  while (WiFi.status() != WL_CONNECTED && connect_try_count < MAX_TRY_CONNECT) {
    delay(500);
    Serial.print(".");
    connect_try_count++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // 接続成功したssid、passwordを覚えておく
    int num_saved_id = preferences.getInt("num_saved_id", 0);
    Serial.print("num_saved_id = ");
    Serial.println(num_saved_id);
    if(num_saved_id >= 2){
      num_saved_id = 0;
      preferences.putInt("num_saved_id", 0);
      Serial.println("reset num_saved_id -> 0");
    }
    if(num_saved_id == 0){
      preferences.putString("ssid_1", map_query["ssid"]);
      preferences.putString("pass_1", map_query["password"]);
      preferences.putInt("num_saved_id", num_saved_id + 1);
      Serial.println("Stored SSID + PASSWORD in storage 1.");
    }else if(num_saved_id == 1){
      preferences.putString("ssid_2", map_query["ssid"]);
      preferences.putString("pass_2", map_query["password"]);
      preferences.putInt("num_saved_id", num_saved_id + 1);
      Serial.println("Stored SSID + PASSWORD in storage 2.");
    }

  } else {
    Serial.println("WiFi connect process time out.");
  } 
  
  server.send_html(200, (char *)success_html);
}

/*-------------------------------------------------*/
// クエリ解析処理
/*-------------------------------------------------*/
static void analyze_query(String query, std::map<String, String> &map_query){
  int pos = 0;
  int index = 0;
  String q_name = "";
  while(true){
    index = query.indexOf("=", pos);
    if (index > 0) {
      q_name = query.substring(pos, index);
      //Serial.print("name = ");
      //Serial.print(q_name);
      pos = index + 1;
      index = query.indexOf("&", pos);
      if (index > 0) {
        //q_valu = query.substring(pos, index);
        map_query[q_name] = query.substring(pos, index);
        //Serial.print(", valu = ");
        //Serial.println(q_valu);
        pos = index + 1;
      } else {
        index = query.indexOf("\r", pos);
        //q_valu = query.substring(pos, index);
        map_query[q_name] = query.substring(pos, index);
        //Serial.print(", valu = ");
        //Serial.println(q_valu);
        break;
      }
    } else {
      break;
    }
  }
  return;
}

/*-------------------------------------------------*/
// 7セグダイナミック点灯処理
/*-------------------------------------------------*/
static void flash_7seg(){
  int c = segDisp.lightNumberByDynamic(ONE_DIGIT_PIN, TWO_DIGIT_PIN, THREE_DIGIT_PIN);
  return;
}

/*-------------------------------------------------*/
// タグ追加文字列取得処理
/*-------------------------------------------------*/
static String getStringAddedTag(String input, String tag){
  return "<" + tag + ">" + input + "</" + tag + ">";
}

/*-------------------------------------------------*/
// 時間文字列取得処理
/*-------------------------------------------------*/
static String getStringTime(int time){
  String adjust_char = "";
  if(time % 60 < 10){
    adjust_char = "0";
  }
  return String(time / 60) + ":" + adjust_char + String(time % 60);
}

/*-------------------------------------------------*/
// 対象順位表示用HTML取得処理
/*-------------------------------------------------*/
static String getHtmlByRank(int rank){
  String html = "";
  rankingData_S ranking_data = rankingManager->getDataByRank(rank);
  html += getStringAddedTag(String(rank), "td");
  html += getStringAddedTag(getStringTime(ranking_data.time), "td");
  html += getStringAddedTag(String(ranking_data.miss), "td");
  html += getStringAddedTag(getStringTime(ranking_data.score), "td");
  html = getStringAddedTag(html, "tr");
  return html;
}

/*-------------------------------------------------*/
// ルートHTML作成処理
/*-------------------------------------------------*/
static void make_root_html(){
  const char *header = "/header.html";
  const char *footer = "/footer.html";
  File html_file;
  uint8_t header_html[1024];
  uint8_t footer_html[1024];
  size_t size;
  size_t total_size = 0;

  //  header.htmlを読み込む
  SPIFFS.begin();
  html_file = SPIFFS.open(header, "r");
  size = html_file.size();
  html_file.read(header_html, size);
  html_file.close();

  memcpy(root_html, header_html, size);
  total_size += size;

  String body_html = "<body><table><caption>ランキング</caption>"
    "<tr><th>順位</th><th>タイム</th><th>ミス回数</th>"
    "<th>ミスペナルティ込みのタイム</th></tr>";
  for(int i = 0; i < rankingManager->RANKING_DATA_NUM; i++){
    body_html += getHtmlByRank(i+1);
  }
  body_html += "</table></body>";

  memcpy(root_html + total_size, body_html.c_str(), body_html.length());
  total_size += body_html.length();

  //  footer.htmlを読み込む
  html_file = SPIFFS.open(footer, "r");
  size = html_file.size();
  html_file.read(footer_html, size);
  html_file.close();

  memcpy(root_html + total_size, footer_html, size);
  total_size += size;
  return;
}

void play(unsigned char Track)
{
  unsigned char play[6] = {0xAA,0x07,0x02,0x00,Track,Track+0xB3};
  serial_voice_module.write(play, 6);
}

void volume( unsigned char vol)
{
  unsigned char volume[5] = {0xAA,0x13,0x01,vol,vol+0xBE};
  serial_voice_module.write(volume, 5);
}