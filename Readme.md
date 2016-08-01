## OpenGL / OpenCV　によるビデオ再生サンプル

 mkdir build  
 cd build  
 cmake ..  
 make  

実行は（シェーダープログラムやビデオファイルを読み込む関係上）  
 cd build  
 ./video     ./video1   など

+ video.cpp :  OpenCV Captureクラスによるビデオ取得とOpenGLによる表示
+ video1.cpp : video.cppを三角形Meshで表現。


