# 開発環境コンテキスト

- デバイス: M5Stack Cardputer (M5StampS3搭載)
- チップセット: ESP32-S3
- フレームワーク: Arduino / PlatformIO
- ライブラリ: M5Cardputer (^1.1.1), M5Unified (^0.2.11)
- 画面解像度: 240x135 (横向き)
- ストレージ: microSDカード (SPI接続)

## オーディオサブシステム

- **Codec:** ES8311 (I2C Addr: 0x18)
  - MCLKはBCLKから内部生成する設定が必要 (Reg 0x01 = 0xB5)。
  - 初期化は `Wire` ライブラリを用いて手動で行う。
- **Amp:** NS4150B
  - **電源制御:** GPIO 46
  - HIGH = ON, LOW = OFF
  - ポップノイズ対策のため、起動時はLOWにし、ストリーム情報受信(`audio_info`)後にHIGHにする。
- **I2S Pinout:**
  - BCLK: 41
  - LRCK: 43
  - DOUT: 42

## 入力デバイス

- **Keyboard:** M5Cardputerライブラリにより抽象化済み。
  - `M5Cardputer.Keyboard.isKeyPressed(key)` で判定。
  - 主要キー: `,`(Vol-), `.`(Vol+), `/`(Next), `Fn`+`BS`(Reset Config)

## センサー・その他

- **IMU:** 実装では未使用。
- **Mic:** 実装では未使用。
- **Wi-Fi:** `Preferences` ライブラリを使用して SSID/Pass を永続化。
