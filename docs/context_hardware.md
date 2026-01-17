# 開発環境コンテキスト

- デバイス: M5Stack Cardputer ADV (M5StampS3A搭載)
- チップセット: ESP32-S3
- フレームワーク: Arduino / PlatformIO
- ライブラリ: M5Cardputer (v1.1.1+), M5Unified (v0.1.16+)
  - **重要:** `M5Cardputer` ライブラリは "Cardputer ADV" をサポートしていると記述あり。
- 画面解像度: 240x135 (横向き)
- オーディオ: 
  - Codec: ES8311 (I2C Addr: 0x18, I2S)
  - Amp: NS4150B
  - Mic: PDM Mic (Standard Cardputer compatible?) or Analog via ES8311? (To be verified)
- センサー:
  - IMU: BMI270 (I2C Addr: 0x68 or 0x69)
  - Keyboard: TCA8418 (I2C) or Shift Register (Standard Cardputer uses Shift Register 74HC138. ADV might use TCA8418?) -> `M5Cardputer` source has `TCA8418` driver, implying ADV might use it.
- 必須設定: `M5Cardputer.begin()` を使用し、ハードウェア初期化を行う。
- GPIOピン配置:
  - 現時点では不明確なため、`M5Unified` の自動設定に依存するか、初回起動時に `M5.Speaker.config()` 等からダンプして特定する。