//
//  main.cpp
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/03/11.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv/highgui.h>

#include "common.h"

using namespace std;
using namespace cv;

// 色のセッティング
// it -> (red, green, blue)
void setColor(const MatIterator_<Vec3b> it, const unsigned char red, const unsigned char green, const unsigned char blue){
    (*it)[0] = blue;
    (*it)[1] = green;
    (*it)[2] = red;
}
void setColor(const MatIterator_<Vec3b> it, const unsigned char luminance){
    setColor(it, luminance, luminance, luminance);
}

// 現在のパターンの表示
void printCurrentPattern(const bool *pattern, const int patternSize){
    for(int i = 0; i < patternSize; ++ i){
        cout << pattern[i];
    }
    cout << endl;
}

// バイナリマップの表示
void printPatternMap(bool *map, const Size* const mapSize){
    for (int y = 0; y < mapSize->height; ++ y) {
        for (int x = 0; x < mapSize->width; ++ x, ++ map) {
            cout << *map;
        }
        cout << endl;
    }
}

// 層数の計算
int calcLayerNumber(const int length){
    // error processing
    if (length < 2) {
        ERROR_PRINT(length);
    }
    
    int layerNum = 1;
    for (int thresh = 2; thresh < 10000; thresh *= 2) {
        if (thresh >= length) {
            break;
        } else {
            ++ layerNum;
        }
    }
    
    return layerNum;
}

// 純二進コードの生成
// pattern      : 生成したコードを入れる場所
// patternSize  : コードの長さ（不変）
// layerNum     : コード層の数(１〜)
void makePureBinaryCode(bool *pattern, const unsigned int patternSize, const unsigned int layerNum){
    // define color
    const bool WHITE = 1;
    
    // divideNum = 2^num
    // 分割数
    int divideNum = 1;
    for(int i = 0; i < layerNum; ++i){
        divideNum *= 2;
    }
    
    // エラー処理
    if(divideNum / 2 > patternSize){
        ERROR_PRINT(divideNum);
        cout << "divideNum(" << divideNum << ") is more than patternSize(" << patternSize << ")" << endl;
        return;
    }else if (divideNum > patternSize){ // if divideNum / 2 < patternSize < divideNum then divideNum is patternSize
        divideNum = patternSize;
    }

    // コード生成
    bool bit = WHITE;
    for (int i = 0, threshStep = 1; i < patternSize; ++ i) {
        // 分割数に応じてビットを反転
        if ((i + 1) > patternSize * threshStep / (double)divideNum ) {
            threshStep ++;
            bit = !bit;
        }
        
        // ビットの決定
        pattern[i] = bit;
    }
}

// グレイコードパターンの生成
// pattern      : パターンを入れる場所
// patternSize  : パターンの長さ
// layerNum     : パターンの層数
void makeGrayCodePattern(bool *pattern, const unsigned int patternSize, const unsigned int layerNum){
    // １層目は純二進コードと同じ
    if (layerNum == 1) {
        makePureBinaryCode(pattern, patternSize, layerNum);
        return;
    }else if (layerNum == 0){
        ERROR_PRINT(layerNum);
        return;
    }
    
    // pbc(純二進コード変数)のイニシャライズ
    bool pbc_current[patternSize], pbc_before[patternSize];
    for (int i = 0; i < patternSize; ++ i) {
        const bool WHITE = 1;
        pbc_current[i] = WHITE;
        pbc_before[i] = WHITE;
    }
    
    // 現在と一つ前の純二進コード取得
    makePureBinaryCode(pbc_current, patternSize, layerNum);
    makePureBinaryCode(pbc_before, patternSize, layerNum - 1);
    
    // グレイコード＝純二進コードの現在と過去の排他的論理和
    for (int i = 0; i < patternSize; ++ i) {
        pattern[i] = pbc_current[i] ^ pbc_before[i];
    }
}

// ポジパターンからネガパターンを生成
// create nega pattern using posi pattern
void makeNegaCodePattern(bool *pattern, const bool *posiPattern, const unsigned int patternSize){
    for (int i = 0; i < patternSize; ++ i) {
        pattern[i] = !posiPattern[i];   // ポジネガ反転
    }
}

// パターンを用いてマップの生成(pattern -> map)
// map          : マップを保持する配列
// pattern      : パターン
// mapSize      : patternMapの大きさ
// direction    : 縞模様の向き（縦か横）
void pattern2map(bool *map, const bool* pattern, const Size* const mapSize, const stripeDirection direction){
    // bool配列に従って色を入れる
    for (int y = 0; y < mapSize->height; ++ y) {
        for (int x = 0; x < mapSize->width; ++ x, ++ map) {
            bool color = BOOL_BLACK;
            
            if (direction == Vertical) {    // 縦縞
                color = *(pattern + x);
            } else {                        // 横縞
                color = *(pattern + y);
            }
            
            *map = color;
        }
    }
}

// パターンマップから画像を生成する
// image    : 生成する画像
// map      : 参照するパターンマップ
// mapSize  : 上二つの大きさ
void map2image(Mat *image, const bool* const map, const Size* const mapSize){
    // init iterator
    MatIterator_<Vec3b> imageItr = image->begin<Vec3b>();
    
    // パターンマップを参照し画像を生成
    for (int y = 0; y < mapSize->height; ++ y) {
        for (int x = 0; x < mapSize->width; ++ x, ++ imageItr) {
            char color = 0;
            const char BLACK = 0, WHITE = 255; // 後で色配列に変更する(スカラーschlor?)
            
            // マップを参照し色を決定
            if (map[y * mapSize->width + x] == BOOL_BLACK) {
                color = BLACK;
            } else {
                color = WHITE;
            }
            
            // 色を画像に入れる
            setColor(imageItr, color);
        }
    }
}

// 引数で与えたコードを横に引いてMatに代入
// make Mat from....
// if内が
void makePatternImage(const bool* pattern, Mat *image, const Size *imageSize, const stripeDirection direction){
    // define black and white
    const unsigned char BLACK = 0, WHITE = 255; // 後で色配列に変更する(スカラーschlor?)
    
    // bool配列に従って色を入れる
    int patternStep = 0, locX = 0;
    for (MatIterator_<Vec3b> imageItr = image->begin<Vec3b>();
         imageItr != image->end<Vec3b>();
         ++ imageItr, ++ locX) {

        // 縦・横どちらの縞模様かを判別
        if (direction == Vertical) {  // 縦縞
            // if patternStep is over imageSize, do initialize
            if ((++ patternStep) >= imageSize->width) {
                patternStep = 0;
            }
        } else {        // 横縞
            if (locX >= imageSize->width) {
                ++ patternStep;
                locX = 0;
            }
        }
        // switing white or black using GrayCode
        unsigned char color = 0;
        if (pattern[patternStep]) {
            color = WHITE;
        } else {
            color = BLACK;
        }
        setColor(imageItr, color);
    }

}

//  アクセスマップに任意の層に従ってビットを挿入する
// accessMap    : 生成するアクセスマップ
// mapSize      : accessMapの大きさ
// bitDepth     : accessMapのビットの深さ
// patternMap   : 参照するパターンマップ
// addBitPos    : 追加するビットの位置
void insertAccessMap(bool* accessMap, const Size* const mapSize, const int bitDepth, const bool* const patternMap, const int addBitPos){
    // 全画素に入れていく
    //for (int y = 0; y < mapSize->height; ++ y) {
        //for (int x = 0; x < mapSize->width; ++ x) {
            //int currentPos = y * mapSize->width + x;
            // accessMap : 各画素にbitDepthビット入っているので，その分進める
            //*(accessMap + addBitPos + currentPos * bitDepth) = *(patternMap + currentPos);
        //}
    //}
}

// ネガポジ画像を用いて二値化画像を生成
// make binary image using posi and nega image
// binaryMap    : 作った二値化マップを代入する変数
// layerNum     : 層番号
// mapSize      : 投影サイズ
// RorC         : 縦縞か横縞かのフラグ(enumにした方がよさげ)
void createBinaryMap(Mat *binaryMap, const Size* const mapSize, const unsigned int layerNum, const stripeDirection direction){
    // error process
    if (binaryMap->rows != mapSize->height || binaryMap->cols != mapSize->width) {
        ERROR_PRINT(binaryMap->rows);
        ERROR_PRINT(binaryMap->cols);
        ERROR_PRINT(mapSize->width);
        ERROR_PRINT(mapSize->height);
        
        return;
    }
    
    // init pattern array
    int patternSize = 0;                                    // パターンの大きさ
    if (direction == Vertical) {
        patternSize = mapSize->width;
    } else {
        patternSize = mapSize->height;
    }
    bool posiPattern[patternSize];                          // グレイコードパターンを入れる配列
    bool patternMap[mapSize->width * mapSize->height];      // 投影するパターンマップ
    Mat projectionImage = Mat::zeros(*mapSize, CV_8UC3);    // 投影画像
    Mat capturedImage;                                      // 撮影画像
    _print2(mapSize->width, mapSize->height);
    _print(layerNum);
    
    // init gray code image
    makeGrayCodePattern(posiPattern, patternSize, layerNum);
    //printCurrentPattern(posiPattern, patternSize);
    //cout << endl;
    pattern2map(patternMap, posiPattern, mapSize, direction);
    //printPatternMap(patternMap, mapSize);
    map2image(&projectionImage, patternMap, mapSize);
    
    // projection and shot posi image
    imshow("projection", projectionImage);  // posi pattern
    waitKey(SLEEP_TIME);
    imshow("projection", ~projectionImage); // nega pattern
    waitKey(SLEEP_TIME);
    //projectionImage.release();
    
    // create access map
    const Size layerSize(calcLayerNumber(mapSize->width), calcLayerNumber(mapSize->height));    // 層の個数
    // (縦層+横層)*全画素数
    const int accessBitNum = layerSize.width + layerSize.height;  // アクセスに必要なビット数
    const int pixelNum = mapSize->width * mapSize->height;        // 全画素数
    int forwardNum = 0;   // 初期ビットの位置
    if (direction == Horizon) {
        forwardNum = layerSize.width;
    } else {
        forwardNum = 0;
    }
    bool accessMapProjector[accessBitNum * pixelNum];   // プロジェクタのアクセスマップ
    //bool accessMapCamera[accessBitNum * pixelNum];      // カメラのアクセスマップ
    for (int i = 0; i < accessBitNum * pixelNum; ++ i) {
        //accessMapProjector[i] = BOOL_BLACK;
    }
    //insertAccessMap(accessMapProjector, mapSize, accessBitNum, patternMap, forwardNum);
    
    //free(accessMapProjector);
    // create binary map
    
}

// ルックアップテーブルの生成
void createLookUpTable(){
    // １．ポジパターン投影
    // ２．ポジパターン撮影
    // ３．ネガパターン投影
    // ４．ネガパターン撮影
    // ５．ポジ画像からネガ画像を引く
    // ６．０を境に二値化する
    // ７．空間コードを付与する
    // ８．次の層に進む
}

// main method
int main(int argc, const char * argv[])
{
    // init camera
    cv::VideoCapture camera(0);
    if( !camera.isOpened() ){
        std::cerr << "ERROR : camera is not opened !!" << std::endl;
        return 1;
    }
    Mat frame;
    camera >> frame;
    Size cameraSize(frame.cols, frame.rows);
    
    // init projection image
    Size projectionSize(PRJ_SIZE_WIDTH, PRJ_SIZE_HEIGHT);
    Mat projectionImage = cv::Mat::zeros(projectionSize, CV_8UC3);
    Size layerSize(calcLayerNumber(projectionSize.width), calcLayerNumber(projectionSize.height));
    
    // init gray code image
    for (int timeStep = 1; timeStep <= layerSize.width; ++ timeStep) {
        createBinaryMap(&projectionImage, &projectionSize, timeStep, Vertical);
    }
    for (int timeStep = 1; timeStep <= layerSize.height; ++ timeStep) {
        createBinaryMap(&projectionImage, &projectionSize, timeStep, Horizon);
    }
    
    return 0;
}

