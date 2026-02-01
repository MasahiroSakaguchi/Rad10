# Rad10 - The Chaos Radio (for Cardputer Adv)

**Rad10** は、M5Stack Cardputer Adv を「ハードボイルドなインターネットラジオ」に変えるファームウェアです。
あなたは能動的に選局することができません。電源を入れるだけで、世界中のどこかの放送局に接続されます。
偶然性（The Chaos）のみが、新しい音楽や言語との出会いをもたらします。

**Rad10** transforms your M5Stack Cardputer Adv into a "Hard-Boiled Internet Radio".
You cannot actively select a station. Just turn it on, and it connects to a broadcast somewhere in the world.
Only "The Chaos" brings you encounters with new music and languages.

## ✨ 特徴 (Features)

* **The Chaos Engine:**
    Radio Browser API を使用し、約100種類のジャンルからランダムに局を選定します。
    Randomly selects a station from about 100 genres using the Radio Browser API.
* **Simple Control:**
    選局ボタンはありません。気に入らなければスキップボタンを押すか、電源を入れ直すだけです。
    No tuning buttons. If you don't like it, just hit the skip button or restart via power switch.
* **Playlog:**
    受信した楽曲情報（アーティスト、曲名など）をSDカードに自動記録します。
    Automatically logs track information (artist, title, etc.) to the SD card.
* **Cardputer Adv Optimized:**
    キーボード操作、ディスプレイ表示、バッテリー管理など、Cardputer Adv に最適化されています。

## 🛠 ハードウェア (Hardware Requirements)

* **Device:** M5Stack Cardputer Adv (M5StampS3)
* **Storage:** microSD Card (Optional, for logging)

## 🚀 インストールとビルド (Installation)

このプロジェクトは [PlatformIO](https://platformio.org/) で開発されています。

1. このリポジトリをクローンします。

    ```bash
    git clone https://github.com/MasahiroSakaguchi/Rad10
    ```

2. VS Code でフォルダを開き、PlatformIO 拡張機能が有効になっていることを確認します。
3. 依存ライブラリは `platformio.ini` に基づき自動的にダウンロードされます。
4. M5Stack Cardputer Adv を接続し、Upload ボタンを押して書き込みます。

This project is developed with [PlatformIO](https://platformio.org/).
Clone the repository, open it in VS Code with PlatformIO, and upload it to your M5Stack Cardputer Adv.

## 📖 使い方 (Usage)

初回起動時、Wi-Fi設定が見つからない場合は「Wi-Fi設定モード」になります。画面の指示に従い、キーボードでSSIDとパスワードを入力してください。

When launched for the first time, if no Wi-Fi settings are found, it enters "Wi-Fi Config Mode". Enter your SSID and Password using the keyboard.

### キー操作 (Key Controls)

| Key | Action |
| :--- | :--- |
| **`/`** | **Skip / Next Station (Chaos Trigger)** |
| **`.`** | Volume Up |
| **`,`** | Volume Down |
| **`Fn` + `Del`** | Reset Wi-Fi Settings (Hold for 3 sec) |

## 📜 ライセンス (License)

このソフトウェアは **CC0 1.0 Universal** の下で公開されています。
著作権を放棄し、パブリックドメインとして提供します。商用・非商用を問わず、複製、改変、再配布が自由に可能です。許可や連絡は一切不要です。

This software is released under the **CC0 1.0 Universal** license.
I waive all copyright and related rights and dedicate this work to the public domain. You are free to copy, modify, distribute, and perform the work, even for commercial purposes, all without asking permission.

See [LICENSE](./LICENSE) for details.
