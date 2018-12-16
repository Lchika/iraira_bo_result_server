# iraira_bo_result_server
- UnityとESP32の連携できました

## 画面サンプル
<img src="https://github.com/Lchika/iraira_bo_result_server/blob/master/doc/example.png" width="400px">

## 概要
- 演出用ソフトを動作させているPCと本プログラムを書き込んだESP32をWi-Fiでつなぐ
- ランキング登録時に、UnityがESP32に対してPOSTリクエストを発行し、ランキングデータを渡す
- ESP32はデータを受け取ったら以下の2つの処理を行う
  - 受け取ったデータをESP32内で管理しているランキングに登録し、ランキング表示用HTMLを更新する
  - 接続されている7セグに表示する値を1加算する

## TODO
- HTMLが素の状態なのでレイアウト等を整える
- 時々POSTリクエストが通らない時があるので原因の調査・修正
