---
title: "📖 Unityエンジニアの視点で学ぶMuJoCo入門"
emoji: "😺"
type: "tech"
topics:
  - "unity"
  - "mujoco"
  - "hakoniwa"
published: true
published_at: "2025-02-02 15:20"
---

**「ロボットの動作をリアルにシミュレーションしたい！」**
そう思ったことはありませんか？

Unityは、リアルタイムレンダリングや直感的な開発環境を活かして、ゲームやVRアプリ開発に最適なプラットフォームです。しかし、物理シミュレーションの精度やロボティクス向けの機能においては、より特化したツールが求められます。

そこで登場するのが、[MuJoCo](https://mujoco.readthedocs.io/en/stable/overview.html#)（Multi-Joint dynamics with Contact） です。
MuJoCoは、高速かつ精度の高い物理シミュレーションを提供し、ロボットの動作や物体間の接触計算に優れています。Google DeepMindをはじめとする研究機関や企業でも広く使われており、強化学習やロボティクス分野での応用が進んでいます。

# こんな人にオススメ！

✅ Unityで物理シミュレーションを試したことがあるが、精度に不満がある人
✅ MuJoCoに興味があるが、どこから始めればよいか分からない人
✅ XMLの書き方を学びつつ、シミュレーションを改造してみたい人

# **🔹 UnityとMuJoCo、何が違うの？**
UnityとMuJoCoは、どちらも物理シミュレーションが可能ですが、その目的が異なります。

|  | **Unity** | **MuJoCo** |
|---|---|---|
| **用途** | ゲーム・VR・インタラクティブコンテンツ | ロボティクス・強化学習・科学シミュレーション |
| **物理エンジン** | NVIDIA PhysX | 専用の高精度エンジン |
| **操作方法** | GUI（エディタでオブジェクト配置） | XMLやスクリプトで定義 |
| **計算精度** | リアルタイム処理向け（近似計算） | 高精度な物理計算 |

つまり、**「見た目やゲーム向けの物理ならUnity、ロボットや高精度なシミュレーションならMuJoCo」** という使い分けが適しています。

より具体的には例えるとこうですかね。
* UnityのPhysXは、剛体シミュレーションのため、関節の柔軟性や接触計算が粗い。
* MuJoCoは、接触力・摩擦の計算精度が高く、逆動力学解析も可能。
* 強化学習において、Unity ML-AgentsではMuJoCoのような精密な接触計算ができない。

今回は、まず **「MuJoCoの基本」** を学び、サンプルXMLファイルを解析しながら、その仕組みを理解していきます。  
MuJoCoがどのように動作するのかを体験しながら、次回以降の「ロボットモデルの作成」に繋げていきましょう！

# **🛠 まずはMuJoCoを触ってみよう！**
それでは、最初にMuJoCoのサンプルシミュレーションを見て、モチベーションを上げましょう！

📺 **参考動画:**  
https://www.youtube.com/watch?v=QnaOydKmV7w

この動画では、MuJoCoを使ったリアルなシミュレーションの例を見ることができます。  
「こんなシミュレーションが作れるのか！」と感じたら、早速MuJoCoを触っていきましょう。


# **🔍 MuJoCoのシミュレーターを使ってみる**
MuJoCoには、**XMLファイルを読み込んでシミュレーションを実行できる「標準のシミュレーター」** が付属しています。  
まずはこれを使って、サンプルXMLを開いてみましょう。

### **💡 手順**
1. **MuJoCoをインストール**すると、下図のアイコンがデスクトップまたはアプリケーションフォルダに追加されます。  
   このアイコンをダブルクリックして、シミュレーターを起動しましょう。  
   * ![](https://storage.googleapis.com/zenn-user-upload/cb60c21be666-20250202.png)

2. シミュレーターが起動すると、下図のようなウィンドウが表示されます。  
   * ![](https://storage.googleapis.com/zenn-user-upload/3341360dbbe1-20250202.png)

3. **サンプルのXMLファイルをドラッグ＆ドロップ** すると、シミュレーションが開始されます！  
   * ![](https://storage.googleapis.com/zenn-user-upload/5347ee38d85f-20250202.png)

---

これで、MuJoCoのシミュレーション環境が整いました！  
次に、**XMLファイルの中身を解析し、シミュレーションの仕組みを理解していきましょう。**


# **✏️ サンプルXMLファイルを理解しよう**

MuJoCoではモデルの定義にXMLファイルを用います。そして、サンプルとして選んだ xml ファイルはバルーンのサンプルです。

このXMLファイルは、MuJoCoでバルーン（風船）を用いたシミュレーションを実現するための設定ファイルです。このXMLファイルは、MuJoCoのシミュレーションにおける基本的な設定を一通り網羅しています。  

- **オプション**でシミュレーションの基本パラメータを設定し、  
- **ビジュアル**で見た目を調整、  
- **デフォルト**で各要素の共通プロパティをまとめ、  
- **アセット**でシーンに使用するリソースを定義し、  
- **ワールドボディ**で実際のシーンのオブジェクトを配置、  
- **腱**で各オブジェクト間の連結を実現しています。

[balloons.xml](https://github.com/google-deepmind/mujoco/blob/main/model/balloons/balloons.xml)：
```xml
<mujoco>
  <option density="1.204" viscosity="1.8e-5" integrator="implicit"/>

  <visual>
    <global elevation="-10"/>
  </visual>

  <default>
    <tendon limited="true" width="0.003" rgba="1 1 1 1"/>
    <geom friction=".2"/>
    <default class="weight">
      <geom rgba=".8 .4 .8 1"/>
      <site rgba=".8 .4 .8 1"/>
    </default>
    <default class="balloon">
      <!--
        0.167 is the MKS density of helium at room temperature.
        Note this does not take into account the mass of the rubber,
        which is likely not insignificant.
      -->
      <geom density="0.167" fluidshape="ellipsoid"/>
      <default class="pink">
        <geom rgba="1 .6 .7 1"/>
        <site rgba="1 .6 .7 1"/>
      </default>
      <default class="blue">
        <geom rgba=".3 .7 .9 1"/>
        <site rgba=".3 .7 .9 1"/>
      </default>
      <default class="green">
        <geom rgba=".4 .9 .5 1"/>
        <site rgba=".4 .9 .5 1"/>
      </default>
      <default class="orange">
        <geom rgba="1 .4 0 1"/>
        <site rgba="1 .4 0 1"/>
      </default>
    </default>
  </default>

  <asset>
    <texture name="grid" type="2d" builtin="checker" width="512" height="512" rgb2="0 0 0" rgb1="1 1 1"/>
    <material name="grid" texture="grid" texrepeat="2 2" texuniform="true" reflectance=".6"/>
  </asset>

  <worldbody>
    <geom name="ground" type="plane" size="5 5 .05" pos="0 0 -.5" material="grid"/>
    <geom name="ramp" type="box" size=".4 .2 .03" pos="0 0 -.4" euler="0 20 0" rgba="1 1 1 1"/>

    <body name="weight" childclass="weight" pos=".3 0 .2">
      <freejoint/>
      <light pos="1 0 3" dir="-1 0 -3" mode="trackcom"/>
      <light pos="-1 0 3" dir="1 0 -3" mode="trackcom"/>
      <!-- The mass of the weight was chosen to be slightly bigger than the total buoyancy of the balloons. -->
      <geom name="weight" type="box" size=".015 .015 .015" mass=".0347"/>
      <site name="weight1" pos=" .013  .013 .013" size="0.005"/>
      <site name="weight2" pos="-.013 -.013 .013" size="0.005"/>
    </body>

    <!-- The gravcomp value of 7.2 is the ratio of air and helium densities at room temperature. -->
    <body name="pink" gravcomp="7.2" pos="-.2 .1 .2" childclass="pink">
      <freejoint />
      <geom name="pink" type="ellipsoid" size=".11 .11 .15"/>
      <geom name="pink_knot" pos="0 0 -.15" size=".02"/>
      <site name="pink" pos="0 0 -.17" size="0.01"/>
    </body>

    <body name="blue" gravcomp="7.2" pos=".1 .1 .2" childclass="blue">
      <freejoint />
      <geom name="blue" type="ellipsoid" size=".12 .12 .15"/>
      <geom name="blue_knot" pos="0 0 -.15" size=".02"/>
      <site name="blue" pos="0 0 -.17" size="0.01"/>
    </body>

    <body name="green" gravcomp="7.2" pos=".1 -.1 .2" childclass="green">
      <freejoint />
      <geom name="green" type="ellipsoid" size=".12 .12 .14"/>
      <geom name="green_knot" pos="0 0 -.14" size=".02"/>
      <site name="green" pos="0 0 -.16" size="0.01"/>
    </body>

    <body name="orange" gravcomp="7.2" pos="-.12 -.12 .2" childclass="orange">
      <freejoint />
      <geom name="orange" type="ellipsoid" size=".12 .12 .13"/>
      <geom name="orange_knot" pos="0 0 -.13" size=".02"/>
      <site name="orange" pos="0 0 -.15" size="0.01"/>
    </body>
  </worldbody>

  <tendon>
    <spatial range="0 0.6">
      <site site="pink"/>
      <site site="weight1"/>
    </spatial>
    <spatial range="0 0.4">
      <site site="blue"/>
      <site site="weight1"/>
    </spatial>
    <spatial range="0 0.3">
      <site site="green"/>
      <site site="weight2"/>
    </spatial>
    <spatial range="0 0.5">
      <site site="orange"/>
      <site site="weight2"/>
    </spatial>
  </tendon>
</mujoco>
```

全体は `<mujoco>` タグで囲まれており、その中でシミュレーションのオプション、ビジュアル設定、デフォルトのプロパティ、リソース（アセット）、シーン（ワールドボディ）、そして連結用の腱（tendon）が定義されています。

詳細は、付録を参照ください。

## ちょっと理解を確かめる

ここまでChatGPTなどを活用して、MuJoCoの基本的な設定やXMLファイルの構造について大体の概要を掴むことができました。ここからは、実際に自分で手を動かして得た実践的な理解をご紹介します。

このサンプルでは、バルーンがあるのに浮かび上がらない現象に気づいたので、パラメータを変更して、実際にバルーンを浮かべてみることにしました。

ポイントは、`gravcomp` です。XML内のコメントには以下のように記載されています：

```
he gravcomp value of 7.2 is the ratio of air and helium densities at room temperature.
```

つまり、これはヘリウムの浮力に関わるパラメータであり、この値を調整すればバルーンの浮力が大きくなるはずです。実際にこの値を変更してみた結果、バルーンが見事に浮かび上がるようになりました。

その結果は、下記の動画でご確認いただけます：

https://www.youtube.com/watch?v=IeSpwzunwZI


# **📝 まとめ**

今回は、**MuJoCoの基本概念とXMLの解析**を行い、実際にバルーンのシミュレーションを修正してみました。  

MuJoCoは、**XMLを使ってモデルを定義し、物理シミュレーションを動かす** という流れが基本です。  
一見シンプルですが、細かいパラメータ調整ができるので、ロボットシミュレーションやAI学習にも応用できます。

次回は、**TurtleBot3のモデルをMuJoCoで作成**し、実際にロボットの物理シミュレーションを動かしてみます！  
「実際のロボットをシミュレートするにはどうすればいい？」を考えながら進めていきましょう。

👉 **次回：「[MuJoCoでTurtleBot3を作ってみる](https://zenn.dev/kanetugu2022/articles/8bad51c8d0f3ef)」** もお楽しみに！

# **付録**

ここでは、[balloons.xml](https://github.com/google-deepmind/mujoco/blob/main/model/balloons/balloons.xml)の細かな設定内容を解説します。

## 1. シミュレーションの基本オプションとビジュアル設定

### `<option>` タグ

```xml
<option density="1.204" viscosity="1.8e-5" integrator="implicit"/>
```

- **density**: シミュレーション内で使用する空気などの流体の密度を指定しています。ここでは 1.204（おそらく kg/m³）と設定されています。
- **viscosity**: 流体の粘性を示すパラメータです。値は 1.8e-5 となっており、現実の空気の粘性に近い値です。
- **integrator**: 数値積分の方法を指定しています。「implicit」は、剛性の高いシミュレーションにおいて安定性を高めるために選ばれる方法です。

### `<visual>` タグ

```xml
<visual>
  <global elevation="-10"/>
</visual>
```

- **global elevation**: カメラの視点やシーン全体の見え方に影響します。ここではシーンを少し見下ろす形にするため、elevation を -10 に設定しています。


## 2. デフォルト設定とクラス

### `<default>` タグ

デフォルト設定では、共通で使用するパラメータやスタイルを定義しています。これにより、各要素で同じ設定を繰り返さずに済むようになっています。

```xml
<default>
  <tendon limited="true" width="0.003" rgba="1 1 1 1"/>
  <geom friction=".2"/>
  ...
</default>
```

- **tendon**: 腱（tendon）のデフォルト設定。`limited="true"` により、腱の伸び縮みが制限され、`width` や `rgba` で見た目の太さや色を指定しています。
- **geom**: 幾何学要素のデフォルト設定。ここでは摩擦係数を 0.2 に設定しており、全体の接触挙動に影響します。

ちなみに、tendonという言葉は最初はイメージしづらかったのですが、実際は下図のような、物体を接続する細い糸のようなものを指しています。実際のシミュレーションでも、この腱が物体同士を柔軟に連結する役割を果たしています。

![](https://storage.googleapis.com/zenn-user-upload/79ce78a6220c-20250202.png)

#### サブクラス設定

デフォルト設定内では、さらにクラス分けがされています。たとえば、`weight` クラスと `balloon` クラスが定義されています。

- **`<default class="weight">`**  
  ここでは、重りに関する見た目の設定が行われています。`<geom>` と `<site>` で、色（rgba）が指定されています。

- **`<default class="balloon">`**  
  風船に関する基本設定です。  
  - **density="0.167"**: ヘリウムの密度に近い値を設定しており、風船の浮力計算に影響します。  
  - **fluidshape="ellipsoid"**: 風船の形状が楕円体として扱われます。  
  さらに、`balloon` クラス内にサブクラス（`pink`, `blue`, `green`, `orange`）が定義され、それぞれの色や外観が細かく設定されています。


## 3. アセット（資産）の定義

### `<asset>` タグ

```xml
<asset>
  <texture name="grid" type="2d" builtin="checker" width="512" height="512" rgb2="0 0 0" rgb1="1 1 1"/>
  <material name="grid" texture="grid" texrepeat="2 2" texuniform="true" reflectance=".6"/>
</asset>
```

- **texture**: 「grid」という名前のテクスチャを定義。ここでは、チェック柄（checker）が指定され、白と黒で構成される画像が生成されます。
- **material**: 上記のテクスチャを利用して、材質「grid」を作成。`texrepeat` や `reflectance` を設定することで、地面などの見た目を制御します。


## 4. ワールドボディ（シーン）の定義

### `<worldbody>` タグ

シーン内のオブジェクト（地面、斜面、風船、重りなど）が定義されます。

```xml
<worldbody>
  <geom name="ground" type="plane" size="5 5 .05" pos="0 0 -.5" material="grid"/>
  <geom name="ramp" type="box" size=".4 .2 .03" pos="0 0 -.4" euler="0 20 0" rgba="1 1 1 1"/>
  ...
</worldbody>
```

- **地面 (`ground`)**  
  - `type="plane"`: 無限平面を表現。
  - `size` と `pos` で大きさや位置を指定。
  - `material="grid"` により、先に定義したチェック柄のマテリアルが適用されます。

- **斜面 (`ramp`)**  
  - `type="box"` として、直方体を表現し、`euler` 属性で回転（ここでは20度）を与えています。

### 各 `<body>` タグ

- **重り (`weight`)**  
  - `childclass="weight"` を指定して、デフォルト設定（重り用）の外観を引き継ぎます。
  - `<freejoint/>` により自由度の高いジョイントを持たせ、シーン内で動かせるようにしています。
  - 内部で `<geom>` や `<site>` タグを使って、実際の形状や参照点（サイト）を定義しています。

- **風船 (`pink`, `blue`, `green`, `orange`)**  
  - それぞれの `<body>` タグで風船を表現。`gravcomp`（重力補償）属性により、空気とヘリウムの浮力の比率が反映されます。
  - 各風船も `<freejoint/>` を持ち、楕円体の形状が `<geom>` タグで定義されています。
  - `<site>` タグは、後の腱の接続ポイントとして使われています。

## 5. 腱（Tendon）の定義

### `<tendon>` タグ

```xml
<tendon>
  <spatial range="0 0.6">
    <site site="pink"/>
    <site site="weight1"/>
  </spatial>
  ...
</tendon>
```

- **spatial タグ**  
  各 `<spatial>` 要素は、2つの `<site>` を接続しており、物理的な拘束（伸び縮みの制限）をシミュレーションします。  
  - `range` 属性で許容される伸び縮みの範囲が設定されています。
  - 例えば、最初の `<spatial>` では、風船（`pink`）と重りの一部（`weight1`）が接続されています。