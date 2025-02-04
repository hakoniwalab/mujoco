---
title: "💻 MuJoCo C言語のAPIで TurtleBot3 を動かしてみる！"
emoji: "😺"
type: "tech"
topics:
  - "unity"
  - "mujoco"
  - "hakoniwa"
published: truee
published_at: "2025-02-04 15:20"
---

今回は、C言語のAPIを使ってプログラムで制御し、TurtleBot3を自動で動かしてみます！ 💻⚙️

# 📌 前回のおさらい
[前回の記事](https://zenn.dev/kanetugu2022/articles/mujoco-tb3-physics) では、TurtleBot3のモデルに以下の要素を追加しました。

✅ freejoint を追加し、重力の影響を受けるようにした
✅ hinge joint を追加し、車輪の回転を可能にした
✅ キャスターを3軸で自由回転できるようにした
✅ motor を追加し、車輪の駆動を制御できるようにした

そして、MuJoCoのシミュレーション上で 「モーターに数値を入力すると動く」 ことを確認しました。

しかし、まだ手動で値を入力する必要がありました。
次は C言語でAPIを直接操作し、プログラムからTurtleBot3を動かす ことに挑戦します！ 🚀

# 🔧 C言語APIを使ったシミュレーションの流れ
MuJoCoの [C言語API](https://mujoco.readthedocs.io/en/stable/APIreference/index.html) を使い、以下の流れでTurtleBot3を制御します。

1. MuJoCoモデル（tb3.xml）を読み込む → mj_loadXML() を使用
2. シミュレーション用のデータ構造を作成 → mj_makeData() を使用
3. APIを使ってモーターに指示を出す → data->ctrl[] を設定
4. ビューアを使って動きを可視化する → viewer_thread() を使用
5. デバッグAPIを活用し、シミュレーションデータを分析する
6. C言語のコードを実際に見ながら、動作を確認する！


# 🏗 C言語でMuJoCoを動かすための準備
すでに TurtleBot3のモデル（[tb3.xml](https://github.com/hakoniwalab/mujoco/blob/main/sources/models/tb3.xml)） は作成済みなので、
今回は MuJoCoのC API を使って C++でシミュレーションコードを実装 します。

まず、以下のようなプロジェクト構成になっています。

```tree
sources
├── CMakeLists.txt
├── build.bash
├── clean.bash
├── cmake-build
├── examples
│   ├── mujoco_capi_call
│   │   ├── CMakeLists.txt
│   │   └── main.cpp  ← シミュレーションのメインコード
│   ├── common
│   │   ├── mujoco_debug.cpp
│   │   ├── mujoco_debug.hpp
│   │   ├── mujoco_viewer.cpp
│   │   └── mujoco_viewer.hpp
├── include
│   └── mujoco
│       ├── mujoco.h
│       ├── mjdata.h
│       ├── mjmodel.h
│       ├── mjrender.h
│       ├── mjui.h
│       ├── ・・・
├── lib
│   └── libmujoco.3.2.7.dylib
├── models
│   └── tb3.xml  ← TurtleBot3のモデルファイル
└── run.bash
```
✅ main.cpp → C++でシミュレーションを実装するコード
✅ mujoco_viewer.cpp → ビューアを使って動作を可視化
✅ mjmodel.h / mjdata.h → MuJoCoのデータ構造

公開リポジトリ：[hakoniwalab/mujoco](https://github.com/hakoniwalab/mujoco/tree/main)

📜 MuJoCoのデータ構造とモデル構造
MuJoCoでは、シミュレーションのデータを mjModel と mjData の2つの構造体 に分けて管理します。

|構造体|	説明|
|---|---|
|mjModel|	シミュレーションの 静的情報（ロボットの構造） を保持|
|mjData|	シミュレーションの 動的な情報（状態、センサーデータ、モーター出力など） を保持|


# 🛠️ 実行環境の準備
- OS: Ubuntu 20.04 / macOS / Windows WSL2 のいずれか
- MuJoCo バージョン: 3.2.7
- 必要なライブラリ: OpenGL, GLFW3
- CMake 3.20 以上

# 🎯 C++でTurtleBot3を動かすコード

以下は、TurtleBot3を動かす C++のサンプルコード です。

```cpp
#include <mujoco/mujoco.h>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include "mujoco_debug.hpp"
#include "mujoco_viewer.hpp"

// MuJoCoのモデルとデータ
static mjData* mujoco_data;
static mjModel* mujoco_model;
static const std::string model_path = "models/tb3.xml";

// シミュレーションスレッド
void simulation_thread(mjModel* model, mjData* data, bool& running_flag, std::mutex& mutex) {
    double simulation_timestep = model->opt.timestep;
    std::cout << "[INFO] Simulation timestep: " << simulation_timestep << " sec" << std::endl;

    while (running_flag) {
        auto start = std::chrono::steady_clock::now();

        {
            std::lock_guard<std::mutex> lock(mutex);
            data->ctrl[0] = 0.2;  // 左モーター
            data->ctrl[1] = 0.5;  // 右モーター
            mj_step(model, data);
        }

        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        double sleep_time = simulation_timestep - elapsed.count();

        if (sleep_time > 0) {
            std::this_thread::sleep_for(std::chrono::duration<double>(sleep_time));
        }
    }
}

int main(int argc, const char* argv[])
{
    // **MuJoCoモデルの読み込み**
    char error[1000];
    std::cout << "[INFO] Loading model: " << model_path << std::endl;
    mujoco_model = mj_loadXML(model_path.c_str(), nullptr, error, sizeof(error));
    if (!mujoco_model) {
        std::cerr << "[ERROR] Failed to load model: " << model_path << "\n" << error << std::endl;
        return 1;
    }
    std::cout << "[INFO] Model loaded successfully." << std::endl;

    // **データの作成**
    std::cout << "[INFO] Creating simulation data." << std::endl;
    mujoco_data = mj_makeData(mujoco_model);
    mj_forward(mujoco_model, mujoco_data);

    // **シミュレーションの実行**
    std::mutex data_mutex;
    bool running_flag = true;
    std::thread sim_thread(simulation_thread, mujoco_model, mujoco_data, std::ref(running_flag), std::ref(data_mutex));

    viewer_thread(mujoco_model, mujoco_data, std::ref(running_flag), std::ref(data_mutex));
    
    running_flag = false;
    sim_thread.join();

    // **リソース解放**
    mj_deleteData(mujoco_data);
    mj_deleteModel(mujoco_model);

    std::cout << "[INFO] Simulation completed successfully." << std::endl;
    return 0;
}
```

## 🔍 ポイント解説
✅ mj_loadXML() でMuJoCoのモデルをロード
✅ mj_makeData() でシミュレーションデータを作成
✅ mj_step() をループ内で実行し、物理シミュレーションを更新
✅ data->ctrl[] でモーターの制御信号を出力
✅ viewer_thread() を起動し、可視化を実施

なお、viewer_thread() は、ビューアを使ってシミュレーションを可視化する関数ですが、メインスレッドで実行する必要があります。
そのため、シミュレーションの実行体は別スレッドで実行し、メインスレッドでビューアを起動しています。

ちなみに、ビューア内でシミュレーションを実行するという設計案もありますが、今回は分離して実装しています。
なぜなら、今後、箱庭シミュレーションやUnityとの連携を考えると、分離しておいた方が柔軟に対応できるからです。

※箱庭シミュレーションとは、複数のシミュレータを連携させてロボットの制御を行うプラットフォームです。
詳しくは[こちら](https://toppers.github.io/hakoniwa/)をご覧ください。

# 🔍 デバッグAPIの活用
MuJoCoには、シミュレーションの状態を分析するためのデバッグAPI が用意されています。
これを利用することで、ロボットの位置や姿勢角、センサーのデータをリアルタイムで監視 できます。

以下、いくつか例を紹介します。

詳細はこちらを参照ください。

https://github.com/hakoniwalab/mujoco/blob/main/sources/examples/common/mujoco_debug.cpp


## 位置座標の表示(ワールド座標系)

```cpp
void print_body_state_by_name(const mjModel* model, const mjData* data, const std::string& body_name) {
    int body_id = mj_name2id(model, mjOBJ_BODY, body_name.c_str());
    if (body_id != -1) {
        std::cout << "[Body] " << body_name 
                  << " | Position: (" << data->xpos[3 * body_id] << ", "
                                      << data->xpos[3 * body_id + 1] << ", "
                                      << data->xpos[3 * body_id + 2] << ")" 
                  << std::endl;
    } else {
        std::cerr << "[ERROR] Body not found: " << body_name << std::endl;
    }
}
```

## 姿勢角の表示（ワールド座標系）
```cpp
void print_body_orientation_by_name_deg(const mjModel* model, const mjData* data, const std::string& body_name) {
    int body_id = mj_name2id(model, mjOBJ_BODY, body_name.c_str());
    if (body_id != -1) {
        double roll, pitch, yaw;
        quat_to_euler(&data->xquat[4 * body_id], roll, pitch, yaw);

        std::cout << "[Body Orientation (deg)] " << body_name 
                  << " | Roll: " << roll * (180.0 / M_PI) << "°"
                  << ", Pitch: " << pitch * (180.0 / M_PI) << "°"
                  << ", Yaw: " << yaw * (180.0 / M_PI) << "°"
                  << std::endl;
    } else {
        std::cerr << "[ERROR] Body not found: " << body_name << std::endl;
    }
}
```

## ジョイントの情報表示

```cpp
void print_joint_state_by_name(const mjModel* model, const mjData* data, const std::string& joint_name) {
    int joint_id = mj_name2id(model, mjOBJ_JOINT, joint_name.c_str());
    if (joint_id != -1) {
        std::cout << "[Joint] " << joint_name 
                  << " | qpos: " << data->qpos[joint_id]
                  << ", qvel: " << data->qvel[joint_id] 
                  << std::endl;
    } else {
        std::cerr << "[ERROR] Joint not found: " << joint_name << std::endl;
    }
}
```

# 🎬 実際に動かしてみよう！

以下の動画では、C++で実装したシミュレーションコードを実行し、TurtleBot3を動かしている様子を OpenGL ビューアで確認できます。
制御としては、左右のモーターにそれぞれ値を入力していますが、片方向に偏っているので、円を描くように動いています。
ちなみに、マウスで操作することで、シミュレーションの視点を変更できます。

```bash
cd sources
bash build.bash
bash run.bash
```

https://www.youtube.com/watch?v=nr54xT_pD4E

✅ C言語のAPIを使ってTurtleBot3を制御できる！
✅ ビューアでリアルタイムの動きを可視化！

# 📝 まとめ
今回は、MuJoCoのC APIを使って、TurtleBot3をプログラム制御 しました！ 🎮✨

✅ mj_loadXML() でモデルを読み込み
✅ mj_makeData() でシミュレーションデータを作成
✅ mj_step() をループ内で実行し、物理シミュレーションを更新
✅ data->ctrl[] でモーターを制御
✅ ビューアを使ってシミュレーションを可視化
✅ デバッグAPIを活用し、シミュレーションデータを分析

これで、C言語APIを使ったMuJoCoの基礎がしっかり理解できました！ 🚀

# 🔥 次回予告
次回は、「OnShape で TurtleBot3 のモデリングをしてみる！」 に挑戦します！ 🛠️✨

💡 「MuJoCo上のTurtleBot3のモデルをもっとリアルに作るには？」
💡 「CADデータからMuJoCoのXMLへ変換するには？」

👉 次回：「OnShape で TurtleBot3 のモデリングをしてみる！」 をお楽しみに！ 🎨💻
