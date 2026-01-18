# flowchart

``` mermaid

graph TD
    %% 起動プロセス
    subgraph Initialization [1. 起動時: setup]
        A[電源ON / リセット] --> B[ハードウェア初期化<br/>画面・スピーカー・I2C]
        B --> C{Wi-Fi設定が<br/>保存されている?}
        C -- No --> D[inputWiFiConfig<br/>キーボードから設定入力]
        D --> D2[設定保存 & 再起動]
        D2 --> A
        C -- Yes --> E[Wi-Fi接続開始]
        E --> F[Audio初期化<br/>ピン設定・音量設定]
        F --> G[playNewStation<br/>最初の局をリクエスト]
    end

    %% メインループ
    subgraph MainLoop [2. 常時実行: loop]
        G --> H[M5.update / M5Cardputer.update<br/>本体とキーボードの状態更新]
        H --> I[audio.loop<br/>★データの受信・解凍・再生を少しずつ進める]
        I --> J{キー入力あり?}
        
        %% ユーザー操作
        J -- "',' or '.'" --> K[音量を増減<br/>audio.setVolume]
        K --> K2[画面更新<br/>updateDisplay]
        
        J -- "'/'" --> L[playNewStation<br/>次の局へ切り替え]
        
        %% 自動更新
        I -.-> M{曲が終了した?<br/>audio_eof_mp3}
        M -- Yes --> L
    end

    %% ラジオ選局ロジック
    subgraph RadioLogic [3. 選局処理: playNewStation]
        L --> N[fetchWorkingStation<br/>1. ジャンルをランダム抽選]
        N --> O[2. APIでラジオ局を検索]
        O --> P{ビットレートは<br/>128kbps以下?}
        P -- No --> N
        P -- Yes --> Q[URLを取得]
        Q --> R[audio.connecttohost<br/>サーバーに接続開始]
        R --> H
    end

    %% スタイル設定
    style I fill:#f96,stroke:#333,stroke-width:2px
    style Initialization fill:#f5f5f5,stroke:#333
    style MainLoop fill:#e1f5fe,stroke:#01579b
    style RadioLogic fill:#fff9c4,stroke:#fbc02d

```
