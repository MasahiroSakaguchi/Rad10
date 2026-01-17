#include <Arduino.h>
#include <M5Cardputer.h>
#include <M5Unified.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Audio.h> // ESP32-audioI2S

// --- Wi-Fi設定 ---
const char *WIFI_SSID = "deco.wifi.x50"; // ★ご自身のSSID
const char *WIFI_PASSWORD = "20230922.kin"; // ★ご自身のパスワード

// --- グローバルオブジェクト ---
Audio audio; // PSRAM非使用

// --- 状態変数 ---
int currentVolume = 12; // 0-21 (Audioライブラリの仕様)
String currentStationName = "Waiting...";
String currentStreamUrl = "";

// --- 関数: ラジオ局取得 ---
String fetchWorkingStation()
{
    const char *tags[] = {
        "pop", "rock", "jazz", "classical", "lofi", "chillout", "ambient", "piano", "80s", "oldies",
        "japan", "anime", "jpop", "bollywood", "country", "blues", "celtic", "chanson", "disco",
        "latino", "salsa", "bossa nova", "reggae", "hawaiian", "news", "soundtrack"
    };
    int tagCount = sizeof(tags) / sizeof(tags[0]);
    int tagIndex = random(0, tagCount);
    String selectedTag = tags[tagIndex];
    M5.Lcd.fillRect(0, 30, 240, 100, BLACK);
    M5.Lcd.setCursor(10, 60);
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.printf("Genre: %s", selectedTag.c_str());
    M5.Lcd.setCursor(10, 80);
    M5.Lcd.printf("Finding...");
    
    WiFiClient client;
    HTTPClient http;
    int maxOffset = 30;
    int offset = random(0, maxOffset);
    String url = "http://all.api.radio-browser.info/json/stations/search?limit=1&order=clickcount&reverse=true&tag=" + selectedTag + "&offset=" + String(offset);
    
    if (!http.begin(client, url)) return "";
    
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET();
    String foundUrl = "";
    if (httpCode == HTTP_CODE_OK)
    {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, http.getStream());
        if (!error && doc.size() > 0)
        {
            foundUrl = doc[0]["url_resolved"].as<String>();
            currentStationName = doc[0]["name"].as<String>();
        }
    }
    http.end();
    return foundUrl;
}

// --- 画面更新 ---
void updateDisplay()
{
    M5.Lcd.fillRect(0, 0, 240, 30, BLACK); // Header
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(5, 5);
    M5.Lcd.printf("Radio ADV | Vol: %d", currentVolume);
    M5.Lcd.fillRect(0, 30, 240, 105, BLACK); // Body
    M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextColor(CYAN);
    M5.Lcd.setTextSize(1);
    String name = currentStationName;
    if (name.length() > 25) name = name.substring(0, 25) + "...";
    M5.Lcd.drawString(name, 120, 70);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.drawString(audio.isRunning() ? "Playing" : "Stopped", 120, 95);
    M5.Lcd.setTextDatum(TL_DATUM);
}

// --- 選局と再生 ---
void playNewStation()
{
    audio.stopSong();
    String url = fetchWorkingStation();
    if (url.length() > 0 && url.startsWith("http"))
    {
        currentStreamUrl = url;
        M5.Lcd.fillRect(0, 30, 240, 100, BLACK);
        M5.Lcd.setCursor(10, 60);
        M5.Lcd.println("Connecting...");
        audio.connecttohost(currentStreamUrl.c_str());
        updateDisplay();
    }
    else
    {
        M5.Lcd.setCursor(10, 60);
        M5.Lcd.setTextColor(RED);
        M5.Lcd.println("Fetch Failed");
        delay(1000);
        playNewStation();
    }
}

// --- ES8311 直接制御関数 ---
void initES8311() {
    // I2Cバスが使える状態か確認し、ES8311(0x18)を起こす
    Wire.beginTransmission(0x18);
    byte error = Wire.endTransmission();
    if (error == 0) {
        Serial.println("ES8311 found! Initializing...");
        
        // 詳細なシーケンス (M5Unifiedのソースコードから正確なパラメータを抽出)
        uint8_t cmds[][2] = {
            {0x00, 0x80}, // 0x00 RESET/ CSM POWER ON
            {0x01, 0xB5}, // 0x01 CLOCK_MANAGER/ MCLK=BCLK (重要!)
            {0x02, 0x18}, // 0x02 CLOCK_MANAGER/ MULT_PRE=3
            {0x0D, 0x01}, // 0x0D SYSTEM/ Power up analog circuitry
            {0x12, 0x00}, // 0x12 SYSTEM/ power-up DAC
            {0x13, 0x10}, // 0x13 SYSTEM/ Enable output to HP drive
            {0x32, 0xBF}, // 0x32 DAC/ DAC volume (0xBF == 0 dB )
            {0x37, 0x08}, // 0x37 DAC/ Bypass DAC equalizer
            {0x17, 0xBF}  // 0x17 ADC/ ADC volume (0xBF == 0 dB )
        };
        
        for (int i=0; i<9; i++) {
            Wire.beginTransmission(0x18);
            Wire.write(cmds[i][0]);
            Wire.write(cmds[i][1]);
            Wire.endTransmission();
            delay(10);
        }
        Serial.println("ES8311 Initialized.");
    } else {
        Serial.println("ES8311 NOT found on I2C!");
    }
}

void setup()
{
    // 1. シリアル開始 (デバッグ用)
    Serial.begin(115200);

    // 2. 電源ホールド & アンプ電源用ピンの設定 (Cardputer ADV / StampS3)
    pinMode(46, OUTPUT);
    digitalWrite(46, HIGH);

    // 3. M5Unified設定 & 初期化
    auto cfg = M5.config();
    cfg.external_spk = true;
    M5Cardputer.begin(cfg, true);
    
    // 4. 画面表示 (生存確認)
    M5.Display.setRotation(1);
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextColor(WHITE);
    M5.Display.setTextSize(2);
    M5.Display.println("Init System...");
    
    // 5. I2C 強制再設定とES8311直接初期化
    Wire.end(); 
    Wire.begin(8, 9); 
    delay(100);
    
    initES8311(); 

    // 6. アンプのハードウェアボリューム最大化
    M5.Speaker.setVolume(255);

    M5.Lcd.println("Wi-Fi Connecting...");
    
    // 6. Wi-Fi接続
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false); // Wi-Fiの省電力モードをオフにする (ストリーミング安定化)
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 30)
    {
        delay(500);
        M5.Lcd.print(".");
        retry++;
    }
    if (WiFi.status() != WL_CONNECTED)
    {
        M5.Lcd.println("\nWi-Fi Failed");
        while (1) delay(100);
    }
    
    // 7. Audioライブラリ設定
    // Cardputer / ADV 共通
    audio.setPinout(41, 43, 42);
    audio.setVolume(currentVolume); 
    audio.setConnectionTimeout(5000, 2000); 
    
    playNewStation();
}

void loop()
{
    M5.update();
    M5Cardputer.update();
    audio.loop();

    bool volChanged = false;
    if (M5Cardputer.Keyboard.isKeyPressed(','))
    {
        if (currentVolume > 0) currentVolume--;
        volChanged = true;
    }
    if (M5Cardputer.Keyboard.isKeyPressed('.'))
    {
        if (currentVolume < 21) currentVolume++;
        volChanged = true;
    }
    if (volChanged)
    {
        audio.setVolume(currentVolume);
        updateDisplay();
        delay(100); 
    }
    if (M5Cardputer.Keyboard.isKeyPressed('/'))
    {
        playNewStation();
    }
}

// --- Audioコールバック ---
void audio_info(const char *info) { Serial.print("info: "); Serial.println(info); }
void audio_showstation(const char *info) { if (String(info).length() > 0) { currentStationName = String(info); updateDisplay(); } }
void audio_eof_mp3(const char *info) { Serial.print("eof_mp3: "); Serial.println(info); playNewStation(); }