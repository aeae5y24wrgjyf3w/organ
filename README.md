# organ
MIDIから波形サンプルを生成

## 使用言語
C

## 開発環境
- Windows 11
- Ubuntu on WSL2
- WSL 2.6.3.0
- Linux Kernel 6.6.87.2-1

## ディレクトリ構成
- [common/](./common/)
  - [io.h](./common/io.h)\
    音源部の共通インターフェース
  - [main.c](./common/main.c)\
    ファイル入出力
- [mid/](./mid/)\
  サンプルMIDIファイルを格納
  - [beethoven_fate.mid](./mid/beethoven_fate.mid)\
    のこぎり波のみを使用するサンプルMIDIファイル
  - [wagner_meistersinger.mid](./mid/wagner_meistersinger.mid)\
    のこぎり波と矩形波の両方を使用するサンプルMIDIファイル
- [saw_squ/](./saw_squ/)\
  のこぎり波や矩形波を直接生成する実装を入れた
  - [saw.c](./saw_squ/saw.c)\
    音源部の実装のひとつ、のこぎり波のみを鳴らす
  - [saw_squ.c](./saw_squ/saw.c)\
    音源部の実装のひとつ、のこぎり波と矩形波の両方を鳴らす
- [sin/](./sin)\
  サイン波を足し合わせて音を生成する実装を入れた
  - [instruments/](./sin/instruments/)\
    音色(倍音構造)を定義するファイルを入れた
    - [instruments.h](./sin/instruments/instruments.h)\
      音色定義関数のインターフェースを定義するファイル
    - [saw.c](./sin/instruments/saw.c)
      音色定義関数、のこぎり波のみを鳴らす
    - [saw_squ.c](./sin/instruments/saw_squ.c)
      音色定義関数、のこぎり波と矩形波の両方を鳴らす
    - [vowels.c](./sin/instruments/vowels.c)
      音色定義関数、男声と女声の"a"の再現
  - [sin.c](./sin/sin.c)\
    音源部の実装のひとつ、サイン波を足し合わせて音を作る
  - [sin.h](./sin/sin.h)
- [LICENSE](./LICENSE)
- [README.md](./README.md)\
  本ファイル
- [説明.md](./説明.md)\
  おもに音源部の詳細な説明が書いてある

## 使用方法
### 使用条件
gccコマンドが利用可能であること

### 準備
1. リポジトリを丸ごとダウンロードします。
2. コンソールを起動し`organ/`に移動します。
3. 目的に応じて以下のいずれかのビルドコマンドを実行します。
   - のこぎり波や矩形波を鳴らしたい
     - 処理速度を重視したい（ただし音数が増えるとむしろ効率は悪化する）
       - のこぎり波のみ使いたい\
         `gcc common/main.c common/io.h saw_squ/saw.c -lm -o organ.exe`
       - のこぎり波と矩形波の両方を使いたい\
         `gcc common/main.c common/io.h saw_squ/saw_squ.c -lm -o organ.exe`
     - 音質を重視したい
       - のこぎり波のみ使いたい\
         `gcc common/main.c common/io.h sin/sin.c sin/sin.h sin/instruments/instruments.h sin/instruments/saw.c -lm -o organ.exe`
       - のこぎり波と矩形波の両方を使いたい\
         `gcc common/main.c common/io.h sin/sin.c sin/sin.h sin/instruments/instruments.h sin/instruments/saw_squ.c -lm -o organ.exe`
   - 母音を再現したい\
     `gcc common/main.c common/io.h sin/sin.c sin/sin.h sin/instruments/instruments.h sin/instruments/vowels.c -lm -o organ.exe`
4. `organ/`配下に`organ.exe`が生成されたことを確認します。

### 実行
1. `organ.exe`にWAVへ変換したいSMFのファイル名をコマンドライン引数として与えて実行します。ファイル名を複数与えるとアプリはそれらを順に読みだしてひとつのWAVを出力します。のこぎり波と矩形波の両方を使う場合のビルドを行った場合は、チャンネル番号が偶数の音がのこぎり波で、奇数の音が矩形波で鳴ります（チャンネル番号が0から始まる場合）。
2. カレントディレクトリにa.wavが生成されたことを確認します（現時点では出力ファイル名は選べません。）。

### 音域、調律、振幅、波形
#### 音域
ノートナンバー24以上の音を鳴らすことができます。

#### 調律
キルンベルガー第一調律に設定してあります。調律は`saw_squ/saw.c`と`saw_squ/saw_squ.c`と`sin/sin.c`のtuning配列で定義されています。

#### 振幅
波形の振幅はMIDIベロシティの平方根に比例します。のこぎり波と矩形波は同じベロシティでは第1倍音が同じ振幅で鳴ります。矩形波のほうのベロシティをのこぎり波の4/3倍にするとだいたい同じ強さで鳴っているように聞こえます。出力WAVファイルでは振幅は正規化されます（最大変位がWAVファイルが実現できる限界の変位に等しくなります。）。
