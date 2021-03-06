//
//  main.cpp
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/03/11.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//

#include <iostream>
#include <GLUT/GLUT.h>
#include "common.h"
#include "myOpenCV.h"

using namespace std;
using namespace cv;


// 現在のパターンの表示
void printCurrentPattern(const bool *pattern, const int patternSize){
    for(int i = 0; i < patternSize; ++ i){
        cout << pattern[i];
    }
    cout << endl;
}

// バイナリマップの表示
void printPatternMap(const bool* const map, const Size* const mapSize){
    for (int y = 0; y < mapSize->height; ++ y) {
        for (int x = 0; x < mapSize->width; ++ x) {
            cout << *(map + y * mapSize->width + x);
        }
        cout << endl;
    }
}

// アクセスマップの表示
void printAccessMap(const bool* const accessMap, const Size* const mapSize, const int mapDepth){
    for (int y = 0; y < mapSize->height; ++ y) {
        for (int x = 0; x < mapSize->width; ++ x) {
            for (int d = 0; d < mapDepth; ++ d) {
                cout << *(accessMap + (y * mapSize->width + x) * mapDepth + d);
            }
            cout << " ";
        }
        cout << endl;
    }
}

// グレイコードに必要なビット数の計算
int calcBitCodeNumber(const int length){
    // error processing
    if (length < 2) {
        ERROR_PRINT(length);
    }
    
    int layerNum = 1;
    for (int thresh = 2; thresh < MAX_WINDOW_SIZE; thresh *= 2) {
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
    }
    
    // patternSizeを2の累乗に落とし込む
    int patternSizeTo2Pow = 0;
    for (int thresh = 2; thresh < MAX_WINDOW_SIZE; thresh *= 2) {
        if (thresh >= patternSize) {
            patternSizeTo2Pow = thresh;
            break;
        }
    }
    
    // コード生成
    bool bit = WHITE;
    for (int i = 0, threshStep = 1; i < patternSize; ++ i) {
        // 分割数に応じてビットを反転
        if ((i + 1) > patternSizeTo2Pow * threshStep / (double)divideNum ) {
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
void pattern2map(bool *map, const bool* const pattern, const Size* const mapSize, const stripeDirection direction){
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

// 画像からパターンマップを生成する
// map      : 生成するパターンマップ
// image    : 参照する画像
// mapSize  : 上二つの大きさ
void image2map(bool* const map, Mat* const image, const Size* const mapSize){
    // init iterator
    MatIterator_<int> imageItr = image->begin<int>();
    
    // パターンマップを参照し画像を生成
    for (int y = 0; y < mapSize->height; ++ y) {
        for (int x = 0; x < mapSize->width; ++ x, ++ imageItr) {
            // 画像を参照し色を決定
            //cout << (int)*imageItr;
            if (*imageItr >= -30) {
                map[y * mapSize->width + x] = BOOL_WHITE;
            } else {
                map[y * mapSize->width + x] = BOOL_BLACK;
            }
        }
        //cout << endl;
    }
}


// ネガポジ画像を用いて二値化画像を生成
// make binary image using posi and nega image
// binaryMap    : 作った二値化マップを代入する変数
// layerNum     : 層番号
// mapSize      : 投影サイズ
// RorC         : 縦縞か横縞かのフラグ(enumにした方がよさげ)
void createBinaryMap(bool *binaryMap, const Size* const mapSize, const unsigned int layerNum, const stripeDirection direction){
    // init pattern array
    int patternSize = 0;                                    // パターンの大きさ
    if (direction == Vertical) {
        patternSize = mapSize->width;
    } else {
        patternSize = mapSize->height;
    }
    bool posiPattern[patternSize];                          // グレイコードパターンを入れる配列
    Mat projectionImage = Mat::zeros(*mapSize, CV_8UC3);    // 投影画像
    Mat capturedImage;                                      // 撮影画像
    _print2(mapSize->width, mapSize->height);
    _print(layerNum);
    
    // init gray code image
    makeGrayCodePattern(posiPattern, patternSize, layerNum);
    //printCurrentPattern(posiPattern, patternSize);
    
    // make binary map
    pattern2map(binaryMap, posiPattern, mapSize, direction);
}

//  アクセスマップに任意の層に従ってビットを挿入する
// accessMap    : 生成するアクセスマップ
// mapSize      : accessMapの大きさ
// bitDepth     : accessMapのビットの深さ
// patternMap   : 参照するパターンマップ
// offsetBit    : 追加するビットの位置
void insertAccessMap(bool* accessMap, const Size* const mapSize, const int bitDepth, const bool* const patternMap, const int offsetBit){
    // 全画素に入れていく
    for (int y = 0; y < mapSize->height; ++ y) {
        for (int x = 0; x < mapSize->width; ++ x) {
            int currentPos = y * mapSize->width + x;
            // offsetBit : 各画素にbitDepthビット入っているので，その分進める
            *(accessMap + offsetBit + currentPos * bitDepth) = *(patternMap + currentPos);
        }
    }
}

// プロカムの空間コードを追加していく
// spatialCodeProjector : プロジェクタの空間コード格納ポインタ
// spatialCodeCamera    : カメラの空間コード格納ポインタ
// patternLayerNum  : 投影パターンの層番号
// offsetBit        : 追加するビットの位置
// direction        : 縞の方向
// camera           : カメラのストリーム
void addSpatialCodeOfProCam(bool* const spatialCodeProjector, bool* const spatialCodeCamera, const Size* const projectorSize, const Size* const cameraSize, const int patternLayerNum, const int offset, const stripeDirection direction, VideoCapture *camera){
    // init
    Mat projectionImage = cv::Mat::zeros(*projectorSize, CV_8UC3);
    Size layerSize(calcBitCodeNumber(projectorSize->width), calcBitCodeNumber(projectorSize->height));
    const int accessBitNum = layerSize.width + layerSize.height;  // アクセスに必要なビット数

    // バイナリマップを作成
    bool *binaryMapProjector = (bool*)malloc(sizeof(bool) * projectorSize->width * projectorSize->height);      // プロジェクタのアクセスマップ
    bool *binaryMapCamera = (bool*)malloc(sizeof(bool) * cameraSize->width * cameraSize->height);      // カメラのアクセスマップ
    createBinaryMap(binaryMapProjector, projectorSize, patternLayerNum, direction);
    
    // バイナリマップから画像を生成
    map2image(&projectionImage, binaryMapProjector, projectorSize);
    
    // 画像を投影・撮影し二値化画像を取得
    // projection and shot posi image
    // posi
    Mat posiImage, negaImage;
    imshow("projection", projectionImage);  // posi pattern
    *camera >> posiImage;
    cvtColor(posiImage, posiImage, CV_BGR2GRAY);    // グレー化
    waitKey(SLEEP_TIME);
    // nega
    imshow("projection", ~projectionImage); // nega pattern
    *camera >> negaImage;
    cvtColor(negaImage, negaImage, CV_BGR2GRAY);    // グレー化
    waitKey(SLEEP_TIME);
    // ポジネガ画像の差分
    /*
     
     ここの差分で多分unsignedになってて０以下が作れない
     作りたい！
     
     */
    // ポジネガ投影の撮影像の差分を作成し二値化する
    Mat diffPosiNega = Mat::zeros(posiImage.rows, posiImage.cols, CV_16SC1);
    printMatPropaty(&posiImage);cout << endl;
    printMatPropaty(&negaImage);cout << endl;
    printMatPropaty(&diffPosiNega);cout << endl;
    subMat(&diffPosiNega, &posiImage, &negaImage);
    //image2map(spatialCodeCamera, &diffPosiNega, cameraSize);
    image2map(binaryMapCamera, &diffPosiNega, cameraSize);
    printPatternMap(binaryMapCamera, cameraSize);
    //imshow("diff image", diffPosiNega);
    //cout << posiImage - negaImage << endl;
    //cout << diffPosiNega << endl;
    
    // プロジェクタとカメラのアクセスマップの生成
    insertAccessMap(spatialCodeProjector, projectorSize, accessBitNum, binaryMapProjector, offset); // プロジェクタの空間コードを追加
    insertAccessMap(spatialCodeCamera, cameraSize, accessBitNum, binaryMapCamera, offset);  // カメラの空間コードを追加
    printAccessMap(spatialCodeProjector, projectorSize, accessBitNum);
    
    // free
    free(binaryMapProjector);
    free(binaryMapCamera);
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
    VideoCapture camera(0);
    if( !camera.isOpened() ){
        std::cerr << "ERROR : camera is not opened !!" << std::endl;
        return 1;
    }
    Mat frame;                                  // カメラ画像
    camera >> frame;
    Size cameraSize(frame.cols, frame.rows);    // カメラの大きさ
    //imshow("camera image", frame);
    
    // init projection image
    Size projectionSize(PRJ_SIZE_WIDTH, PRJ_SIZE_HEIGHT);
    Mat projectionImage = cv::Mat::zeros(projectionSize, CV_8UC3);
    Size layerSize(calcBitCodeNumber(projectionSize.width), calcBitCodeNumber(projectionSize.height));
    const int accessBitNum = layerSize.width + layerSize.height;  // アクセスに必要なビット数
    const int pixelNumProjector = projectionSize.width * projectionSize.height;        // 全画素数
    const int pixelNumCamera = cameraSize.width * cameraSize.height;        // 全画素数

    // (縦層+横層)*全画素数
    bool *grayCodeMapProjector = (bool*)malloc(sizeof(bool) * accessBitNum * pixelNumProjector);// プロジェクタのグレイコードマップ
    bool *grayCodeMapCamera = (bool*)malloc(sizeof(bool) * accessBitNum * pixelNumCamera);      // カメラのグレイコードマップ
    memset(grayCodeMapProjector, 0, sizeof(bool) * accessBitNum * pixelNumProjector);
    memset(grayCodeMapCamera, 0, sizeof(bool) * accessBitNum * pixelNumCamera);
    
    // 縦横の縞模様を投影しグレイコードをプロジェクタ，カメラ双方に付与する
    int offset = 0; // 初期ビットの位置
    for (int timeStep = 1; timeStep <= layerSize.width; ++ timeStep, ++ offset) {
        addSpatialCodeOfProCam(grayCodeMapProjector, grayCodeMapCamera, &projectionSize, &cameraSize, timeStep, offset, Vertical, &camera);
    }
    for (int timeStep = 1; timeStep <= layerSize.height; ++ timeStep, ++ offset) {
        addSpatialCodeOfProCam(grayCodeMapProjector, grayCodeMapCamera, &projectionSize, &cameraSize, timeStep, offset, Horizon, &camera);
    }
    
    // プロカム間のアクセスマップを作る
    Point *accessMapCam2Pro = (Point*)malloc(sizeof(Point) * pixelNumCamera);// カメラからプロジェクタへのアクセスマップ
    initPoint(accessMapCam2Pro, pixelNumCamera);
    
    glutInit(&argc, argv);
    glutCreateWindow(argv[0]);
    glutDisplayFunc(display);
    glutMainLoop();
    
    // アクセスマップの解放
	free(grayCodeMapProjector);
	free(grayCodeMapCamera);
    
    return 0;
}

