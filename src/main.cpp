#include <Arduino.h>
#include <M5Cardputer.h>
#include <M5Unified.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Audio.h>
#include <Preferences.h> // NVS用ライブラリ

// --- グローバルオブジェクト ---
Audio audio; 
Preferences preferences;

// --- 状態変数 ---
int currentVolume = 12;
String currentStationName = "Waiting...";
String currentStreamUrl = "";
String wifiSSID = "";
String wifiPass = "";

// --- 関数プロトタイプ宣言 ---
String fetchWorkingStation();
void updateDisplay();
void playNewStation();
void initES8311();
void inputWiFiConfig();

// --- 簡易テキスト入力UI ---
String inputString(const char* label, bool isPassword = false) {
    String buffer = "";
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.println(label);
    
    // カーソル位置などの管理用
    int cursorX = 10;
    int cursorY = 40;

    // 前回のキー状態をクリアするために少し待つ
    delay(500); 

    while (true) {
        M5Cardputer.update();
        
        // 入力中の文字列を表示
        M5.Lcd.fillRect(0, cursorY, 240, 30, BLACK);
        M5.Lcd.setCursor(cursorX, cursorY);
        M5.Lcd.setTextColor(WHITE);
        if (isPassword) {
            String mask = "";
            for (int i=0; i<buffer.length(); i++) mask += "*";
            M5.Lcd.print(mask + "_");
        } else {
            M5.Lcd.print(buffer + "_");
        }

        if (M5Cardputer.Keyboard.isPressed()) {
            // Enterキーで確定
            if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)) {
                return buffer;
            }
            // Backspaceキーで削除
            if (M5Cardputer.Keyboard.isKeyPressed(KEY_BACKSPACE)) {
                if (buffer.length() > 0) {
                    buffer.remove(buffer.length() - 1);
                }
            }
            // その他の文字入力
            // Cardputerライブラリの仕様に合わせ、押されたキーを探す
            // 英数字、記号などをスキャン
            // (注: CardputerライブラリのKeyboardは一度に複数のキー状態を持つため、
            //  本来はキーマップ変換が必要だが、簡易的にアルファベットと数字をチェックする)
            
            // keysState() は押されているキーのリストを返すが、使い方が複雑なので
            // ここでは簡易的に実装済みの文字チェックを行うか、
            // ライブラリのヘルパーがあれば使う。
            // 残念ながらCardputerライブラリは char 変換を直接提供していない場合があるため、
            // 愚直だが主要なキーをチェックする。
            
            //数字
            for (char c = '0'; c <= '9'; c++) {
                if (M5Cardputer.Keyboard.isKeyPressed(c)) buffer += c;
            }
            // アルファベット (小文字のみ対応、Shiftは今回は簡易版として無視または別途実装要)
            // 実用上、パスワードに大文字がある場合はShiftキーのロジックが必要だが、
            // 今回は小文字+数字+一部記号に限定する。(Shiftキーの実装は複雑になるため)
            for (char c = 'a'; c <= 'z'; c++) {
                if (M5Cardputer.Keyboard.isKeyPressed(c)) {
                    // Shiftが押されていれば大文字にする処理を追加
                    // Cardputerには左Shiftしかないので左のみチェック
                    if (M5Cardputer.Keyboard.isKeyPressed(KEY_LEFT_SHIFT)) {
                        buffer += (char)(c - 32);
                    } else {
                        buffer += c;
                    }
                }
            }
            // 記号 (よく使うもの)
            if (M5Cardputer.Keyboard.isKeyPressed('.')) buffer += ".";
            if (M5Cardputer.Keyboard.isKeyPressed(',')) buffer += ",";
            if (M5Cardputer.Keyboard.isKeyPressed('-')) buffer += "-";
            if (M5Cardputer.Keyboard.isKeyPressed('_')) buffer += "_";
            if (M5Cardputer.Keyboard.isKeyPressed('@')) buffer += "@";
            if (M5Cardputer.Keyboard.isKeyPressed('!')) buffer += "!";
            
            // チャタリング防止
            delay(200);
        }
    }
}

// --- Wi-Fi設定画面 ---
void inputWiFiConfig() {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(RED);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.println("Wi-Fi Setup Mode");
    delay(2000);

    String newSSID = inputString("Enter SSID:");
    String newPass = inputString("Enter Password:", true);

    // 保存
    preferences.putString("ssid", newSSID);
    preferences.putString("pass", newPass);

    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.setCursor(10, 50);
    M5.Lcd.println("Saved!");
    M5.Lcd.println("Rebooting...");
    delay(2000);
    ESP.restart();
}

String fetchWorkingStation() {
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
    if (httpCode == HTTP_CODE_OK) {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, http.getStream());
        if (!error && doc.size() > 0) {
            foundUrl = doc[0]["url_resolved"].as<String>();
            currentStationName = doc[0]["name"].as<String>();
        }
    }
    http.end();
    return foundUrl;
}

void updateDisplay() {
    M5.Lcd.fillRect(0, 0, 240, 30, BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(5, 5);
    M5.Lcd.printf("Radio ADV | Vol: %d", currentVolume);
    M5.Lcd.fillRect(0, 30, 240, 105, BLACK);
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

void playNewStation() {
    audio.stopSong();
    String url = fetchWorkingStation();
    if (url.length() > 0 && url.startsWith("http")) {
        currentStreamUrl = url;
        M5.Lcd.fillRect(0, 30, 240, 100, BLACK);
        M5.Lcd.setCursor(10, 60);
        M5.Lcd.println("Connecting...");
        audio.connecttohost(currentStreamUrl.c_str());
        updateDisplay();
    } else {
        M5.Lcd.setCursor(10, 60);
        M5.Lcd.setTextColor(RED);
        M5.Lcd.println("Fetch Failed");
        delay(1000);
        playNewStation();
    }
}

void initES8311() {
    Wire.beginTransmission(0x18);
    byte error = Wire.endTransmission();
    if (error == 0) {
        Serial.println("ES8311 found! Initializing...");
        uint8_t cmds[][2] = {
            {0x00, 0x80}, {0x01, 0xB5}, {0x02, 0x18}, {0x0D, 0x01},
            {0x12, 0x00}, {0x13, 0x10}, {0x32, 0xBF}, {0x37, 0x08}, {0x17, 0xBF}
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

void setup() {
    Serial.begin(115200);

    // 1. ハードウェア初期化
    pinMode(46, OUTPUT);
    digitalWrite(46, HIGH);
    
    auto cfg = M5.config();
    cfg.external_spk = true;
    M5Cardputer.begin(cfg, true);
    
    M5.Display.setRotation(1);
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextColor(WHITE);
    M5.Display.setTextSize(2);

    // 2. I2C & Audio初期化
    Wire.end(); 
    Wire.begin(8, 9); 
    delay(100);
    initES8311();
    M5.Speaker.setVolume(255);

    // 3. Wi-Fi設定読み込み
    preferences.begin("rad10-config", false); // "rad10-config"という名前空間を開く
    wifiSSID = preferences.getString("ssid", "");
    wifiPass = preferences.getString("pass", "");

    // 設定がない場合は設定モードへ
    if (wifiSSID == "") {
        inputWiFiConfig(); // ここで入力させて再起動する
    }

    // 4. Wi-Fi接続
    M5.Lcd.println("Wi-Fi Connecting...");
    M5.Lcd.setTextSize(1);
    M5.Lcd.println(wifiSSID); // 接続先を表示
    M5.Lcd.setTextSize(2);

    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.begin(wifiSSID.c_str(), wifiPass.c_str());

    int retry = 0;
    // 30回(約15秒)試行
    while (WiFi.status() != WL_CONNECTED && retry < 30) {
        delay(500);
        M5.Lcd.print(".");
        retry++;
        // Gキーを押しながら起動など、強制的に設定モードに入る手段があると便利だが、
        // 今回は接続失敗時に設定モードに入るようにする
    }

    if (WiFi.status() != WL_CONNECTED) {
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.setTextColor(RED);
        M5.Lcd.println("Connect Failed!");
        delay(2000);
        // 接続失敗 -> 設定モードへ
        inputWiFiConfig();
    }

    // 5. 接続成功
    M5.Lcd.println("\nWi-Fi Connected!");
    delay(1000);

    audio.setPinout(41, 43, 42);
    audio.setVolume(currentVolume); 
    audio.setConnectionTimeout(5000, 2000); 
    
    playNewStation();
}

void loop() {
    M5.update();
    M5Cardputer.update();
    audio.loop();

    bool volChanged = false;
    if (M5Cardputer.Keyboard.isKeyPressed(',')) { if (currentVolume > 0) currentVolume--; volChanged = true; }
    if (M5Cardputer.Keyboard.isKeyPressed('.')) { if (currentVolume < 21) currentVolume++; volChanged = true; }
    if (volChanged) {
        audio.setVolume(currentVolume);
        updateDisplay();
        delay(100); 
    }
    if (M5Cardputer.Keyboard.isKeyPressed('/')) {
        playNewStation();
    }
}
