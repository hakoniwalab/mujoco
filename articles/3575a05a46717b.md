---
title: "🚀 MuJoCoでTurtleBot3を動かしてみる！～物理特性とアクチュエータの追加～"
emoji: "📝"
type: "tech"
topics:
  - "turtlebot3"
  - "mujoco"
  - "hakoniwa"
published: true
published_at: "2025-02-04 05:49"
---

# 📌 前回の振り返り
[前回の記事](https://zenn.dev/kanetugu2022/articles/8bad51c8d0f3ef)では、TurtleBot3のモデルをMuJoCoで作成しましたが、まだ動作しませんでした。

その理由は、以下の点が不足していたからです。
❌ モーターがない → 車輪が駆動しない
❌ 関節の設定がない → 車輪が回転できない
❌ キャスターが固定されている → 正しい動きにならない

今回は、このモデルに 物理特性とアクチュエータを追加し、実際に動かせるようにしていきます！ 🚗💨

# ⚙️ 物理特性の設定
現在のモデルでは、ボディや車輪は定義されていますが、物理特性の設定が不足しているため、シミュレーション上で正しく動作しません。

ここからは、以下の物理パラメータを追加していきます！

✅ 重力の影響を受けるようにする → freejoint を追加
✅ 車輪が回転できるようにする → hinge joint を追加
✅ キャスターを自由に回転できるようにする → hinge joint を3方向に追加

以下が、物理特性を追加したバージョンのXMLで、各種物理特性を、bodyの下に追加してます。

```xml
  <worldbody>
    <geom name="ground" type="plane" size="5 5 .05" pos="0 0 -.5" material="grid"/>

    <!-- 本体（ベース） -->
    <body name="tb3_base" pos="0 0 0.05" childclass="orange">
      <freejoint/>
      <geom name="base" type="box" size="0.13 0.13 0.16" mass="1.0"/>
      
      <!-- 左車輪 -->
      <body name="left_wheel" pos="0.1 0.14 -0.12" euler="90 0 0" childclass="pink">
        <joint name="left_wheel_hinge" type="hinge" axis="0 0 -1" damping="0.1"/>
        <geom name="left_wheel_geom" type="cylinder" size="0.065 0.009" density="500" friction="1.1"/>
      </body>

      <!-- 右車輪 -->
      <body name="right_wheel" pos="0.1 -0.14 -0.12" euler="90 0 0" childclass="green">
        <joint name="right_wheel_hinge" type="hinge" axis="0 0 -1" damping="0.1"/>
        <geom name="right_wheel_geom" type="cylinder" size="0.065 0.009" density="500" friction="1.1"/>
      </body>

      <!-- キャスター -->
      <body name="back_castor" pos="-0.12 0 -0.175" childclass="blue">
        <joint name="castor_yaw" type="hinge" axis="0 0 1" damping="0.1"/>
        <joint name="castor_pitch" type="hinge" axis="0 1 0" damping="0.1"/>
        <joint name="castor_roll" type="hinge" axis="1 0 0" damping="0.1"/>
        <geom name="castor" type="sphere" size="0.02" density="500" friction="0.5"/>
      </body>
    </body>
  </worldbody>
```
🔍 追加したポイント
✅ freejoint を付けて重力の影響を受けるように設定
✅ hinge joint を追加して、車輪の回転を可能に
✅ キャスターには 3つの hinge を追加し、自由回転を実現



# 🏗 シミュレーションの実行
この設定を適用してシミュレーションを実行すると……

![](https://storage.googleapis.com/zenn-user-upload/3670ef85646e-20250202.png)

💡 重力の影響を受けて、ボディが地面で止まるようになりました！ 🎉
しかし、まだ動きません。次に、モーターを追加していきます！

# ⚡ アクチュエータの定義と制御
ここまでで物理特性は設定されましたが、モーターがないため、車輪が回転しません。

そこで、MuJoCoの <actuator> タグを使って モーター駆動 を追加します！

```xml
  <actuator>
    <!-- 左モーター（初期値5.0で回転） -->
    <motor name="left_motor" joint="left_wheel_hinge" ctrllimited="true" ctrlrange="-10 10"  gear = "1.0"/>

    <!-- 右モーター（初期値5.0で回転） -->
    <motor name="right_motor" joint="right_wheel_hinge" ctrllimited="true" ctrlrange="-10 10"  gear = "1.0"/>
  </actuator>

```
🛠 追加したポイント
✅ 各モーターを<motor>タグで定義
✅ joint属性で、対応する車輪のhinge jointを指定
✅ ctrlrangeでモーターの最大回転範囲を指定（-10 〜 10）
✅ gear="1.0" でギア比を設定

# 🎬 シミュレーションの実行と結果の可視化
🏁 論よりRUN!! 実際に動かしてみましょう！

🎥 シミュレーションの様子（YouTube）

https://www.youtube.com/watch?v=MOoypcylzuA

動画のポイント🎯
✅ Control の各モーターの値を変えると、TurtleBot3が動く！
✅ 物理特性の設定 + アクチュエータの追加 により、リアルなシミュレーションが実現！


# 📝 まとめ
今回は、TurtleBot3を動かすために 物理特性とアクチュエータを追加 しました！ 🚀

✅ freejoint を追加し、重力の影響を受けるように
✅ hinge joint で車輪を回転可能に
✅ キャスターを3軸で自由回転できるように
✅ モーターを追加し、車輪の駆動を制御

その結果、MuJoCo上でTurtleBot3が動くようになりました！ 🎮🎉

# 🔥 次のステップ
これで基本的なロボットシミュレーションが実現しました！
しかし、今の状態では モーターの制御を手動で入力 しています。

次回は、C言語のAPIを使って、TurtleBot3をプログラムで制御する 方法を学びます！ 💻⚙️

🔍 なぜC言語で制御するのか？
✅ MuJoCoはC言語で記述されたライブラリ なので、直接制御することで高速なシミュレーションが可能！
✅ Pythonとは違い、リアルタイム制御や組み込み開発にも応用できる
✅ ROSとの統合や、実際のロボットに近い制御を実現する第一歩！

👉 次回：「MuJoCo C言語のAPIで TurtleBot3 を動かしてみる！」 をお楽しみに！ 🚀