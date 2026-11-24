[日本語版は下にあります](#日本語)

## Moeru Attack

Moeru Attack started from a question: what if the maid café "moe moe kyun" were a combat system? The player inputs three poses (Moe, Kyun, Nyan), and each sequence of three triggers a different technique.

Join the maid Aki in defeating the enemy maids on her way to make her maid cafe the most popular in Akihabara!

## How to run

A packaged build can be downloaded from the [releases](https://github.com/napronap/meido/releases) section of this repository (Windows only). Alternatively, to build it yourself follow the instructions below.

Needs UE 5.6

1. Modify `UE56_DIR` in `vars.bat.example` to the root directory of your UE 5.6 installation
2. Rename `vars.bat.example` to `vars.bat`
3. Run `build.bat`
4. Run `editor.bat` or just open `meido_5_6.uproject`

To create a Windows package run `package-windows.bat` (creates a shipping build by default)

The project uses the [VRM4U](https://github.com/ruyo/VRM4U) third party plugin (already present in this repo)

## How to play

### Controls

| Input | Action |
| --- | --- |
| WASD | Move |
| LMB | Attack |
| RMB | Dash |
| Wheel button | Toggle Lock-on on/off |
| E/Q | Change Lock-on target |
| 1 | Pose "Moe" |
| 2 | Pose "Kyun" |
| 3 | Pose "Nyan" |
| Esc | Pause |
| K | Stop AI (debug, currently shipped in the shipping build though) |

### Match rules

Defeat the 10 enemy maids that come to attack you. You win after you defeat them all, and then you can restart to try again.

There are three different pose combos you can perform. Input the poses in the correct order to launch special attacks. If you input an incorrect sequence, you will upset Aki!

| Input (M=Moe, K=Kyun, N=Nyan) | Result |
| --- | --- |
| MMK | Oishiku nare~: summon a giant omurice to hit your enemies |
| MKN | Maid beam: shoot a beam to defeat villain maids |
| NNN | Nyan punch~!: jump forward in a cat attack to deal damage |

## AI usage disclosure

Previous AI generated code was removed and it was later used for review and cleanup of code smells; and to assist me in the process of learning.

---

## 日本語

### 萌えるアタック

萌えるアタックは「メイドカフェの『萌え萌えキュン』が戦闘システムだったら？」という発想から生まれたゲームです。プレイヤーは3つのポーズ（萌え・キュン・ニャン）を入力し、3つの組み合わせごとに異なる技が発動します。

メイドのアキと一緒に、行く手を阻む敵メイドたちを倒して、アキのメイドカフェを秋葉原でいちばんの人気店にしよう！

### 実行方法

ビルド済みのパッケージ（Windowsのみ）は、このリポジトリの[リリース](https://github.com/napronap/meido/releases)からダウンロードできます。自分でビルドする場合は、以下の手順に従ってください。

UE 5.6が必要です。

1. `vars.bat.example` の `UE56_DIR` を、UE 5.6のインストール先のルートディレクトリに変更する
2. `vars.bat.example` を `vars.bat` にリネームする
3. `build.bat` を実行する
4. `editor.bat` を実行するか、`meido_5_6.uproject` を直接開く

Windows向けのパッケージを作成するには `package-windows.bat` を実行してください（デフォルトではShippingビルドが作成されます）。

本プロジェクトはサードパーティ製プラグイン [VRM4U](https://github.com/ruyo/VRM4U) を使用しています（このリポジトリに同梱済み）。

### 遊び方

#### 操作方法

| 入力 | アクション |
| --- | --- |
| WASD | 移動 |
| 左クリック | 攻撃 |
| 右クリック | ダッシュ |
| ホイールクリック | ロックオンのオン/オフ |
| E/Q | ロックオン対象の切り替え |
| 1 | ポーズ「萌え」 |
| 2 | ポーズ「キュン」 |
| 3 | ポーズ「ニャン」 |
| Esc | 一時停止 |
| K | AI停止（デバッグ用。ただし現状はShippingビルドにも含まれています） |

#### ルール

襲ってくる10人の敵メイドを倒しましょう。全員倒すと勝利で、その後リスタートしてもう一度挑戦できます。

ポーズのコンボは3種類あります。正しい順番でポーズを入力すると必殺技が発動します。間違った順番で入力すると、アキが怒ってしまいます！

| 入力（M=萌え、K=キュン、N=ニャン） | 結果 |
| --- | --- |
| MMK | おいしくなーれ：巨大オムライスを召喚して敵を攻撃 |
| MKN | メイドビーム：ビームを放って悪役メイドを倒す |
| NNN | ニャンパンチ～！：猫のように前へ飛びかかってダメージを与える |

### AIの使用について

以前AIで生成したコードは削除しました。その後AIは、コードのレビューとコードスメルの整理、そして自分の学習の補助として使用しています。
