---
title: "箱庭で TurtleBot3 を動かす：ROBOTIS の xacro/URDF から MuJoCo + Godot へ"
emoji: "🤖"
type: "tech"
topics: ["hakoniwa", "mujoco", "godot", "ros", "wsl2"]
published: true
---

# はじめに

箱庭 Getting Started に、Windows + WSL2 向け QuickStart を追加しました。

この記事では、単なる起動手順ではなく、今回の QuickStart が何を体験できる入口なのかを、少し設計寄りに紹介します。

箱庭 Getting Started のリポジトリはこちらです。

https://github.com/hakoniwalab/hakoniwa-getting-started

Windows + WSL2 向け QuickStart はこちらです。

https://github.com/hakoniwalab/hakoniwa-getting-started/blob/main/docs/quick-start-win.md

# 箱庭は「シミュレータ」ではなく「接続基盤」

箱庭は、単体のシミュレータではありません。

MuJoCo、Godot、Python、C++ など、それぞれが得意な役割を担うアプリケーションを、一つのシミュレーション世界として接続するためのプラットフォームです。

今回の QuickStart では、TurtleBot3 を題材にして、次のような構成を体験できます。

- MuJoCo: 物理シミュレーションと LiDAR センサーモデル
- Godot: 3D 可視化
- Python: ゲームパッド入力と LiDAR 可視化
- 箱庭コア: 時刻同期と PDU によるデータ連携

それぞれを一つの巨大なアプリケーションに押し込むのではなく、得意な場所で動かし、箱庭がそれらを接続します。

![MuJoCo + Godot + LiDAR visualization](/images/hakoniwa-tb3-getting-started-win-wsl2.png)

# 今回の TurtleBot3 は ROBOTIS の xacro / URDF が起点

今回の TurtleBot3 は、ROBOTIS が公開している xacro / URDF 資産を起点にしています。

箱庭では、この ROS ロボットモデル資産をもとに、MuJoCo 用の物理モデル、Godot 用の表示モデル、Python アプリとの PDU 連携へ展開します。

ここで重要なのは、**「ROS 資産を使うこと」と「ROS 実行環境に依存すること」を分けて考えられる**点です。

ROS の世界には、URDF、xacro、IDL、メッセージ定義など、多くの有用な資産があります。一方で、シミュレーション環境を作るたびに ROS 実行環境そのものを前提にすると、セットアップや依存関係が重くなりがちです。

箱庭では、ROS の資産を活かしながら、MuJoCo、Godot、Python などを組み合わせた別の実行環境へ展開できます。

# MuJoCo と Godot の役割分担

MuJoCo は、物理シミュレーションに強いエンジンです。剛体、ジョイント、接触、センサーなどを扱うには非常に強力です。

一方で、リッチな 3D 表現やインタラクティブな可視化については、ゲームエンジンである Godot の方が扱いやすい場面があります。

今回の構成では、MuJoCo が物理を担当し、Godot が可視化を担当します。

```text
ROBOTIS xacro / URDF
        ↓
箱庭のツール群
        ↓
MuJoCo 用モデル      Godot 用表示モデル
        ↓                    ↓
  物理シミュレーション      3D 可視化
        └────── 箱庭 PDU / 時刻同期 ──────┘
```

物理と可視化を無理に一つのエンジンに閉じ込めるのではなく、それぞれの得意分野を活かして接続する、という考え方です。

# PDU でデータをつなぐ

箱庭では、プロセス間のデータ連携に PDU を使います。

今回の QuickStart では、ゲームパッド入力から生成された Twist コマンドが MuJoCo 側へ送られ、MuJoCo 側で計算されたロボット状態や LiDAR スキャン結果が、Godot や Python の可視化アプリへ渡されます。

```text
ゲームパッド
    ↓
Python アプリ
    ↓ Twist PDU
MuJoCo
    ↓ JointState / LiDAR PDU
Godot / Python 可視化
```

このように、各プロセスは PDU を介して接続されます。

# 時刻同期が「勝手についてくる」

複数のプロセスを接続すると、次に問題になるのが時刻です。

物理シミュレーション、可視化、制御アプリ、センサー処理が別々のプロセスで動く場合、それぞれが勝手なタイミングで進むと、再現性のあるシミュレーションを作るのが難しくなります。

箱庭では、各アセットが箱庭時刻に従って進むため、MuJoCo、Godot、Python アプリが同じシミュレーション時刻の上で連携できます。

今回の QuickStart では、この仕組みを意識しなくても、TurtleBot3 を動かす体験の中で自然に利用できます。

# なぜ Getting Started が必要だったのか

箱庭は、PDU、時刻同期、MuJoCo、Godot、ROS IDL、URDF など、複数の要素を扱います。

そのため、「何ができるのか」は説明できても、「最初にどこから触ればよいのか」が分かりにくくなりがちでした。

今回の Getting Started は、TurtleBot3 をゲームパッドで動かすという一つの体験に絞ることで、箱庭の全体像を手で触れるようにしたものです。

まずは動かす。
その後で、MuJoCo、Godot、PDU、時刻同期、URDF 変換の意味を理解する。

その順番で箱庭に入れるようにすることが、今回の QuickStart の目的です。

# Windows + WSL2 を選んだ理由

箱庭は、複数のシミュレータやアプリケーションを接続するプラットフォームです。そのため、「どこから使えばよいのか分かりにくい」という声をいただくことがあります。

そこで今回は、ユーザーが手元で試しやすい Windows 環境を入口にしました。一方で、箱庭はコマンドライン操作やスクリプト起動との相性がよいため、起動や確認作業には WSL2 を使う構成にしています。

この QuickStart では、Windows 上で TurtleBot3 を動かしながら、MuJoCo、Godot、Python アプリが箱庭を介して接続される様子を、最短ルートで体験できるようにしています。

# デモ動画

TurtleBot3 を MuJoCo で動かし、Godot 側で可視化している様子です。

https://www.youtube.com/watch?v=sCWhi5JLmdE

# 実際の手順

実際の手順は GitHub 側の QuickStart にまとめています。

https://github.com/hakoniwalab/hakoniwa-getting-started/blob/main/docs/quick-start-win.md

# おわりに

今回の QuickStart は、Windows + WSL2 上で TurtleBot3 を動かすための手順であると同時に、箱庭が目指している「接続基盤」としての考え方を体験する入口でもあります。

ROBOTIS の xacro / URDF 資産を起点に、MuJoCo で物理を動かし、Godot で可視化し、Python アプリで操作・センサー可視化を行う。

それらが箱庭を介して一つのシミュレーション世界として接続される。

まずは TurtleBot3 をゲームパッドで動かしながら、その感覚を試してみてください。

