## OpenGL / OpenCV　によるビデオ再生サンプル

 mkdir build  
 cd build  
 cmake ..  
 make  

実行は（シェーダープログラムやビデオファイルを読み込む関係上）  
 cd build  
 ./video     ./video1   など

### video.cpp :  OpenCV Captureクラスによるビデオ取得とOpenGLによる表示
### video1.cpp : video.cppを三角形Meshで表現。
### video3.cpp : 複数のビデオストリームをそれぞれ独立したtextureとして扱う。

### video4.cpp : morph.frag テクスチャのmorphingサンプル

+       ./video4 で　../vtex1.txt の定義を読み込んで画像を変形させる
+       m キー でmorphing をトグル
+       d キー でデバッグ情報の表示をトグル

### video5.cpp : video4.cppで使うmorphing の定義を作成する。

背景上でマウスドラッグで　特徴線を追加
ビデオ画像に合わせてマウスドラッグで　特徴線を追加
この２本が特徴線の対応になる。同様にして特徴線を追加していく。

+       m キー でmorphing をトグル
+       d キー でデバッグ情報の表示をトグル
+       BACKSPACE で最後の特徴線を削除
+       s キー でファイル保存
+       l キー でファイルロード

### video6.cpp render-to-texture sample

video4.cpp の描画内容をtextureに書き込み、それを表示する。



　　　


