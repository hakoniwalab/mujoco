---
title: "💻 MuJoCo C言語のAPIで TurtleBot3 を動かしてみる！"
emoji: "😺"
type: "tech"
topics:
  - "unity"
  - "mujoco"
  - "hakoniwa"
published: false
published_at: "2025-02-05 00:00"
---

さー、今回は、C言語のAPIを使ってプログラムで制御し、TurtleBot3を自動で動かしてみますよ！ 💻⚙️

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

1. MuJoCoモデル（tb3.xml）を読み込む
2. シミュレーションを開始する
3. APIを使ってモーターに指示を出す
4. ビューアを使って動きを可視化する
5. C言語のコードを実際に見ながら、動作を確認していきましょう！


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
公開リポジトリ：[hakoniwalab/mujoco](https://github.com/hakoniwalab/mujoco/tree/main)

# 🎯 C++でTurtleBot3を動かすコード

まずは、C++でTurtleBot3を動かすコードを見ていきましょう。

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
✅ MuJoCoのモデル（tb3.xml）を読み込む → mj_loadXML()
✅ データを作成し、初期状態を計算 → mj_makeData() & mj_forward()
✅ 別スレッドでシミュレーションを実行 → simulation_thread()
✅ APIを使ってモーターに指示を出す → data->ctrl[0] = 0.2; data->ctrl[1] = 0.5;
✅ ビューアを使って可視化する → viewer_thread()

# 🎬 実際に動かしてみよう！

```bash
cd sources
bash build.bash
bash run.bash
```

https://www.youtube.com/watch?v=nr54xT_pD4E
