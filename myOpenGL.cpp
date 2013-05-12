//
//  myOpenGL.cpp
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/05/11.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//

#include "myOpenGL.h"

void myInitGlut(int argc, const char * argv[]){
    //glutInit(&argc, argv); //[1]:初期化
    glutCreateWindow(argv[0]);//[2]:ウィンドウ作成
    /** [3]:コールバック関数　登録 **/
    glutDisplayFunc(display); //display関数を登録
}
// 描画関数
void display(void){
    
}

