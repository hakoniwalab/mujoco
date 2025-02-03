---
title: "⚒️ MuJoCoでTurtleBot3を作ってみる！～ロボットシミュレーションの第一歩～"
emoji: "🙌"
type: "tech"
topics:
  - "ros"
  - "turtlebot3"
  - "mujoco"
  - "hakoniwa"
published: true
published_at: "2025-02-03 15:42"
---

# 🔙 前回のまとめ

[前回の記事](https://zenn.dev/kanetugu2022/articles/1bc075b4a32aa5)では、MuJoCoの基本を学び、バルーンのシミュレーションを通して XMLの記述方法 や 物理特性の調整 を体験しました。最初は浮かなかったバルーンを gravcomp を調整することで、バルーンを浮上させることができましたよね。

今回は、このXMLを改造して、実際のロボット（TurtleBot3）をMuJoCoで再現する ことに挑戦します！

# 🎯 こんな人にオススメ！
✅ Unityでロボットシミュレーションを試したことがあるが、物理精度に不満がある人
✅ MuJoCoを使って、実際のロボットモデルを作ってみたい人
✅ ROSを使っていて、TurtleBot3のシミュレーションに興味がある人
✅ モーター制御や関節の設定など、MuJoCoの応用を学びたい人

# 🚀 ハローワールド
新しいプログラミング言語を学ぶ際に「Hello, World!」を出力するように、  
ロボットシミュレーションでも最初のステップとして「モデルを作成し、動かす」ことはとっても大事だと思います。

今回のゴールは **MuJoCo上でTurtleBot3を作成できるようになること!**！ 🎯

🐢 **TurtleBot3とは？**  
[TurtleBot3](https://e-shop.robotis.co.jp/list.php?c_id=93) は、シンプルな構造の移動ロボットです。  
- 🏠 **ボディ**（メインフレーム）  
- 🔄 **2つの車輪**（左右独立駆動）  
- 🎱 **1つのキャスター**（後方サポート）  

このシンプルさが、MuJoCoでロボットモデルを作成するための最初のステップとして最適です。

🛠 **MuJoCoでTurtleBot3を再現する方法**  
MuJoCoでこのモデルを再現する際には、  
- 📦 ボディ → **剛体（body）**  
- 🔄 車輪 → **回転関節（joint）**  
- ⚙️ モーター → **アクチュエータ（actuator）**  
といった要素を設定します。

# 🛠 ロボットモデルの定義

🎯 **目標:**  
TurtleBot3を **MuJoCoのXML** で定義し、シミュレーション上にモデルを構築すること！  

---

## 📝 **TurtleBot3 の XML 定義**
以下が、自分なりに TurtleBot3 を表現した MuJoCo の XML です。
**寸法などはズレているかもしれませんが、だいたいこんな感じだと思います。**

```xml
<mujoco>
  <! 前回と同じものは省略 -->

  <worldbody>
    <geom name="ground" type="plane" size="5 5 .05" pos="0 0 -.5" material="grid"/>

    <!-- 本体（ベース） -->
    <body name="tb3_base" pos="0 0 0.05" childclass="orange">
      <geom name="base" type="box" size="0.13 0.13 0.16" mass="1.0"/>
      
      <!-- 左車輪 -->
      <body name="left_wheel" pos="0.1 0.14 -0.12" euler="90 0 0" childclass="pink">
        <geom name="left_wheel_geom" type="cylinder" size="0.065 0.009" density="500" friction="1.1"/>
      </body>

      <!-- 右車輪 -->
      <body name="right_wheel" pos="0.1 -0.14 -0.12" euler="90 0 0" childclass="green">
        <geom name="right_wheel_geom" type="cylinder" size="0.065 0.009" density="500" friction="1.1"/>
      </body>

      <!-- キャスター -->
      <body name="back_castor" pos="-0.12 0 -0.175" childclass="blue">
        <geom name="castor" type="sphere" size="0.02" density="500" friction="0.5"/>
      </body>
    </body>
  </worldbody>
</mujoco>
```

## 📌 モデルの構造
### 🔹 tb3_base（本体）

中央に位置するボディで、ロボットのメインフレームを担う
childclass="orange" でデフォルトのオレンジ色を適用

### 🔹 left_wheel / right_wheel（左右の車輪）

左右独立駆動する差分モーターを持つ車輪
euler="90 0 0" で回転を設定し、正しく回るように配置

### 🔹 back_castor（キャスター）

後方に配置し、移動時のバランスを取るための部品
type="sphere" でシンプルに球体として定義し、適切な摩擦を設定

## 🏗 シミュレーション結果

ロボットのモデルをMuJoCo上に作成すると、以下のようになります！

![](https://storage.googleapis.com/zenn-user-upload/8e3bb117a766-20250202.png)

が…… まったく動かない 😨💦

これは、物理特性の設定が不足している ためです！ ⚠️

🚨 足りないもの
✅ モーター（actuator）が定義されていない → 駆動力がない
✅ 車輪が固定されていない → 回転軸の設定が必要
✅ キャスターの摩擦設定が不十分 → 正しい挙動をしない可能性あり


# **📝 まとめ**
今回は、**MuJoCoのXMLを使って、TurtleBot3の基本モデルを作成** しました！ 🚀  

MuJoCoでは、  
✅ **ボディ（剛体）**、✅ **車輪（回転関節）**、✅ **キャスター（補助部品）** などを  
**XMLで定義し、物理シミュレーションを構築する** ことができます。  

一見シンプルですが、細かい物理パラメータを調整することで、よりリアルなロボットシミュレーションが可能になります。 🔍  

しかし、今回のモデルでは**まだ動きません**…！😨💦  
👉 その理由は、**モーターや関節の設定が足りない** からでした。  

---

## **🔥 次回予告！**
次回は、**TurtleBot3にモーター（actuator）を追加し、実際に動かせるように** します！ 🚗💨  

**「ロボットをシミュレートするには何が必要か？」** を考えながら進めていきましょう！  

👉 **次回：「[MuJoCoでTurtleBot3を動かしてみる！](https://zenn.dev/kanetugu2022/articles/3575a05a46717b)」** をお楽しみに！ 🎮⚙️  