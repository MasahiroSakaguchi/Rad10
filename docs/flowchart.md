# flowchart

``` mermaid

graph TD
    %% 起動プロセス
    subgraph Initialization [1. 起動時: setup]
        A[電源ON] --> B[GPIO46 OFF<br/>アンプ電源切断]
        B --> C[M5Cardputer & ES8311初期化]
        C --> D{Wi-Fi設定あり?}
        D -- No / Connect Fail --> E[inputWiFiConfig<br/>キーボード入力画面]
        E --> F[設定保存 & 再起動]
        D -- Yes --> G[Wi-Fi接続 & 時刻同期]
        G --> H[Audio初期化<br/>I2S設定]
        H --> I[playNewStation<br/>最初の局をリクエスト]
    end

    %% メインループ
    subgraph MainLoop [2. 常時実行: loop]
        I --> J[M5Cardputer.update<br/>キー状態更新]
        J --> K[audio.loop<br/>ストリーミング処理]
        K --> L{キー入力あり?}
        
        %% ユーザー操作
        L -- "',' (Down) / '.' (Up)" --> M[音量変更]
        M --> N[画面更新]
        
        L -- "'/'" --> O[playNewStation<br/>次の局へ切り替え]
        
        L -- "Fn + Backspace (3s)" --> P[Wi-Fi設定削除 & 再起動]
        
        %% 自動更新
        K -.-> Q{曲終了/エラー?}
        Q -- Yes --> O
        
        %% 画面定期更新
        K -.-> R[1分ごとにバッテリー表示更新]
    end

    %% ラジオ選局ロジック
    subgraph RadioLogic [3. 選局処理: playNewStation]
        O --> S[fetchWorkingStation<br/>ジャンル抽選 & API検索]
        S --> T{Bitrate <= 128k?}
        T -- No --> S
        T -- Yes --> U[URL取得]
        U --> V[audio.connecttohost]
    end

    %% バックグラウンド処理
    subgraph Callbacks [4. イベントコールバック]
        V -.-> W[audio_info<br/>メタデータ受信]
        W --> X{アンプOFF?}
        X -- Yes --> Y[GPIO46 HIGH<br/>アンプ電源投入]
        
        W -.-> Z[audio_showstreamtitle<br/>曲名更新]
        Z --> AA{新規タイトル?}
        AA -- Yes --> AB[SDカードへログ保存<br/>playlog.csv]
    end

    %% スタイル設定
    style K fill:#f96,stroke:#333,stroke-width:2px
    style Initialization fill:#f5f5f5,stroke:#333
    style MainLoop fill:#e1f5fe,stroke:#01579b
    style RadioLogic fill:#fff9c4,stroke:#fbc02d
    style Callbacks fill:#e8f5e9,stroke:#2e7d32

```
