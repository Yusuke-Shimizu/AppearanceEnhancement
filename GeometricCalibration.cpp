//
//  GeometricCalibration.cpp
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/05/23.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//

#include "GeometricCalibration.h"
#include "common.h"

using namespace std;
using namespace cv;

// コンストラクタ
GeometricCalibration::GeometricCalibration(void) {
}
GeometricCalibration::GeometricCalibration(ProCam* _procam){
    setProCam(_procam);
}

///////////////////////////////  set method ///////////////////////////////
bool GeometricCalibration::setProCam(ProCam* procam){
    m_procam = procam;
    return true;
}

///////////////////////////////  get method ///////////////////////////////
ProCam* GeometricCalibration::getProCam(void){
    return m_procam;
}

///////////////////////////////  print method ///////////////////////////////

// 現在のパターンの表示
void GeometricCalibration::printCurrentPattern(const bool* const pattern, const int patternSize){
    for(int i = 0; i < patternSize; ++ i){
        cout << *(pattern + i);
    }
    cout << endl;
}

// バイナリマップの表示
void GeometricCalibration::printPatternMap(const bool* const map, const Size* const mapSize){
    for (int y = 0; y < mapSize->height; ++ y) {
        for (int x = 0; x < mapSize->width; ++ x) {
            cout << *(map + y * mapSize->width + x);
        }
        cout << endl;
    }
}

// アクセスマップの表示
void GeometricCalibration::printAccessMap(const bool* const accessMap, const Size* const mapSize, const int mapDepth){
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
int GeometricCalibration::calcBitCodeNumber(const int length){
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
// 上のテスト
void GeometricCalibration::test_calcBitCodeNumber(void){
    for (int i = 1; i < 1025; ++ i) {
        int layer = calcBitCodeNumber(i);
        _print2(i, layer);
    }
}

// 純二進コードの生成
// pattern      : 生成したコードを入れる場所
// patternSize  : コードの長さ（不変）
// layerNum     : コード層の数(１〜)
void GeometricCalibration::makePureBinaryCode(bool *pattern, const unsigned int patternSize, const unsigned int layerNum){
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
    bool bit = BOOL_BLACK;
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

// 未完成
bool GeometricCalibration::getNthBit(const int num, const int Nth){
    return true;
}

// グレイコードパターンの生成
// pattern      : パターンを入れる場所
// patternSize  : パターンの長さ
// layerNum     : パターンの層数
void GeometricCalibration::makeGrayCodePattern(bool *pattern, const unsigned int patternSize, const unsigned int layerNum){
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
        pbc_current[i] = BOOL_WHITE;
        pbc_before[i] = BOOL_WHITE;
    }
    
    // 現在と一つ前の純二進コード取得
    makePureBinaryCode(pbc_current, patternSize, layerNum);
    makePureBinaryCode(pbc_before, patternSize, layerNum - 1);
    
    // グレイコード＝純二進コードの現在と過去の排他的論理和
    for (int i = 0; i < patternSize; ++ i) {
        pattern[i] = pbc_current[i] ^ pbc_before[i];
    }
}
// 座標値で生成する方法
void GeometricCalibration::makeGrayCodePattern2(bool *pattern, const unsigned int patternSize, const unsigned int layerNum){
    // １層目は純二進コードと同じ
    if (layerNum == 1) {
        makePureBinaryCode(pattern, patternSize, layerNum);
        return;
    }else if (layerNum == 0){
        ERROR_PRINT(layerNum);
        return;
    }

    // patternを全走査し，N番目とN-1番目のビットの排他的論理和を用いる
    for (int i = 0; i < patternSize; ++ i) {
        pattern[i] = getNthBit(i, layerNum - 1) ^ getNthBit(i, layerNum);
    }
}

// ポジパターンからネガパターンを生成
// create nega pattern using posi pattern
void GeometricCalibration::makeNegaCodePattern(bool *pattern, const bool *posiPattern, const unsigned int patternSize){
    for (int i = 0; i < patternSize; ++ i) {
        pattern[i] = !posiPattern[i];   // ポジネガ反転
    }
}

// パターンを用いてマップの生成(pattern -> map)
// map          : マップを保持する配列
// pattern      : パターン
// mapSize      : patternMapの大きさ
// direction    : 縞模様の向き（縦か横）
void GeometricCalibration::pattern2map(bool *map, const bool* const pattern, const Size* const mapSize, const stripeDirection direction){
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
void GeometricCalibration::map2image(Mat *image, const bool* const map, const Size* const mapSize){
    // init iterator
    MatIterator_<Vec3b> imageItr = image->begin<Vec3b>();
    
    // パターンマップを参照し画像を生成
    for (int y = 0; y < mapSize->height; ++ y) {
        for (int x = 0; x < mapSize->width; ++ x, ++ imageItr) {
            char color = 0;
            
            // マップを参照し色を決定
            if (map[y * mapSize->width + x] == BOOL_BLACK) {
                color = CHAR_BLACK;
            } else {
                color = CHAR_WHITE;
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
void GeometricCalibration::image2map(bool* const map, Mat* const image, const Size* const mapSize){
    // init iterator
    MatIterator_<int> imageItr = image->begin<int>();
    
    // パターンマップを参照し画像を生成
    for (int y = 0; y < mapSize->height; ++ y) {
        for (int x = 0; x < mapSize->width; ++ x, ++ imageItr) {
            // 画像を参照し色を決定
            if (*imageItr >= BINARY_THRESH) {
                map[y * mapSize->width + x] = BOOL_WHITE;
            } else {
                map[y * mapSize->width + x] = BOOL_BLACK;
            }
        }
    }
}

// アクセスマップから画像を生成する
// image    : 生成する画像
// map      : 参照するパターンマップ
// mapSize  : 上二つの大きさ
void GeometricCalibration::accessMap2image(Mat *image, const Point* const accessMap, const Size& mapSize, const Size& maxSize){
    // error processing
    Size imageSize(image->cols, image->rows);
    if ( mapSize != imageSize) {
        cerr << "map size is different from image size" << endl;
        ERROR_PRINT2(mapSize, imageSize);
        exit(-1);
    }
    
    // パターンマップを参照し画像を生成
    MatIterator_<Vec3b> imageItr = image->begin<Vec3b>();
    for (int y = 0; y < mapSize.height; ++ y) {
        for (int x = 0; x < mapSize.width; ++ x, ++ imageItr) {
            char color = 0;
            
            // マップを参照し色を決定
            color = (accessMap + x + y * mapSize.width)->x * UCHAR_MAX / maxSize.width;
            
            // 色を画像に入れる
//            _print2("before", *imageItr);
            setColor(imageItr, color);
//            _print2("after", *imageItr);
        }
    }
}

// ネガポジ画像を用いて二値化画像を生成
// make binary image using posi and nega image
// binaryMap    : 作った二値化マップを代入する変数
// layerNum     : 層番号
// mapSize      : 投影サイズ
// RorC         : 縦縞か横縞かのフラグ(enumにした方がよさげ)
void GeometricCalibration::createBinaryMap(bool *binaryMap, const Size* const mapSize, const unsigned int layerNum, const stripeDirection direction){
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
    
    // init gray code image
    makeGrayCodePattern(posiPattern, patternSize, layerNum);
    
    // make binary map
    pattern2map(binaryMap, posiPattern, mapSize, direction);
}

//  アクセスマップに任意の層に従ってビットを挿入する ok
// accessMap    : 生成するアクセスマップ
// mapSize      : accessMapの大きさ
// bitDepth     : accessMapのビットの深さ
// patternMap   : 参照するパターンマップ
// offsetBit    : 追加するビットの位置
void GeometricCalibration::insertAccessMap(bool* accessMap, const Size* const mapSize, const int bitDepth, const bool* const patternMap, const int offsetBit){
    // 全画素に入れていく
    for (int y = 0; y < mapSize->height; ++ y) {
        for (int x = 0; x < mapSize->width; ++ x) {
            int currentPos = y * mapSize->width + x;
            // offsetBit : 各画素にbitDepthビット入っているので，その分進める
            *(accessMap + offsetBit + currentPos * bitDepth) = *(patternMap + currentPos);
        }
    }
}
// 上のテスト
void GeometricCalibration::test_insertAccessMap(void){
    // init
    const Size size(10, 4);
    const int depth = 4;
    bool* access = (bool*)malloc(sizeof(bool) * size.width * size.height * depth);
    memset(access, 0, sizeof(bool) * size.width * size.height * depth);
    bool* pattern = (bool*)malloc(sizeof(bool) * size.width * size.height);
    memset(pattern, 1, sizeof(bool) * size.width * size.height);
    pattern[1 + 2 * size.width] = 0;
    pattern[2 + 2 * size.width] = 0;
    
    // insert
    /*cout << "insert to " << endl;
     printPatternMap(pattern, &size);
     cout << "before" << endl;
     printAccessMap(access, &size, depth);
     insertAccessMap(access, &size, depth, pattern, 1);
     cout << "after" << endl;
     printAccessMap(access, &size, depth);
     */
    // free
    free(access);
    free(pattern);
}

// 入力された投影像を投影・撮影を行い，撮影像を出力する
void GeometricCalibration::captureProjectionImage(Mat* const captureImage, const Mat* const projectionImage, VideoCapture *videoStream){
    // 投影
//    imshow(W_NAME_GEO_PROJECTOR, *projectionImage);  // posi pattern
//    waitKey(SLEEP_TIME);
//    
//    // 撮影
//    Mat image;
//    for (int i = 0; i < CAPTURE_NUM; ++ i) {
//        *videoStream >> image;
//    }
//    *captureImage = image.clone();
//    //imshow(W_NAME_GEO_CAMERA, *captureImage);
//	//cvMoveWindow(W_NAME_GEO_CAMERA, projectionImage->rows, 0);
//	waitKey(SLEEP_TIME);
//    
    ProCam* l_procam = getProCam();
    l_procam->captureFromLight(captureImage, *projectionImage);
}

int num = 0;
// プロカムの空間コードを追加していく
// spatialCodeProjector : プロジェクタの空間コード格納ポインタ
// spatialCodeCamera    : カメラの空間コード格納ポインタ
// patternLayerNum  : 投影パターンの層番号
// offsetBit        : 追加するビットの位置
// direction        : 縞の方向
// camera           : カメラのストリーム
void GeometricCalibration::addSpatialCodeOfProCam(bool* const spatialCodeProjector, bool* const spatialCodeCamera, const Size* const projectorSize, const Size* const cameraSize, const int patternLayerNum, const int offset, const stripeDirection direction, VideoCapture *videoStream){
    // init
    Mat projectionPosiImage = cv::Mat::zeros(*projectorSize, CV_8UC3);  // ポジ画像
    
    // 投影用バイナリマップを作成
    bool *binaryMapProjector = (bool*)malloc(sizeof(bool) * projectorSize->width * projectorSize->height);      // プロジェクタのアクセスマップ
    bool *binaryMapCamera = (bool*)malloc(sizeof(bool) * cameraSize->width * cameraSize->height);      // カメラのアクセスマップ
    createBinaryMap(binaryMapProjector, projectorSize, patternLayerNum, direction);
    
    // バイナリマップから投影画像を生成
    map2image(&projectionPosiImage, binaryMapProjector, projectorSize);
    
    // projection and shot posi image
    // posi
    Mat posiImage, negaImage;
    waitKey(SLEEP_TIME);
    captureProjectionImage(&posiImage, &projectionPosiImage, videoStream);
    // nega
    Mat projectionNegaImage = ~projectionPosiImage;     // ネガ画像
    captureProjectionImage(&negaImage, &projectionNegaImage, videoStream);
    projectionPosiImage.release();  // free
    projectionNegaImage.release();  // free
    
    // カラー画像からグレー画像へ変換
    cvtColor(posiImage, posiImage, CV_BGR2GRAY);
    cvtColor(negaImage, negaImage, CV_BGR2GRAY);
	
    // ポジネガ撮影像を符号有り１６ビットに拡張 ok
	Mat posiImage16bit, negaImage16bit;
    posiImage.convertTo(posiImage16bit, CV_16SC1);
    negaImage.convertTo(negaImage16bit, CV_16SC1);
    posiImage.release();    // free
    negaImage.release();    // free
    
    // ポジネガ撮影像の差分 ok
    Mat diffPosiNega16s = posiImage16bit - negaImage16bit;
    
    // 差分画像の二値化
    image2map(binaryMapCamera, &diffPosiNega16s, cameraSize);
    map2image(&diffPosiNega16s, binaryMapCamera, cameraSize);
    
    // 差分画像の表示 ok
    imshow16s("diff image", &diffPosiNega16s);
    cvMoveWindow("diff image", 0, 0);
    
    // 差分画像の書き出し
    Mat diffPosiNega8u = Mat::zeros(diffPosiNega16s.rows, diffPosiNega16s.cols, CV_8UC1);
    convertMatDepth16sTo8u(&diffPosiNega8u, &diffPosiNega16s);
    ostringstream oss;
    oss << "diffImage" << (num++) << ".png";
    imwrite(oss.str().c_str(), diffPosiNega8u);
    cout << "write diffImage.png" << endl;
    
    // プロジェクタとカメラのアクセスマップの生成
    const Size layerSize(calcBitCodeNumber(projectorSize->width), calcBitCodeNumber(projectorSize->height));  // コード層の数
    const int accessBitNum = layerSize.width + layerSize.height;  // アクセスに必要なビット数
    insertAccessMap(spatialCodeProjector, projectorSize, accessBitNum, binaryMapProjector, offset); // プロジェクタの空間コードを追加
    insertAccessMap(spatialCodeCamera, cameraSize, accessBitNum, binaryMapCamera, offset);  // カメラの空間コードを追加
    
    //printAccessMap(spatialCodeProjector, projectorSize, accessBitNum);
    //cout << "------" << endl;
    
    // free
    free(binaryMapProjector);
    free(binaryMapCamera);
}

// コードを分割して代入する
void GeometricCalibration::divideCode(bool* const codeX, bool* const codeY, const bool* const originalCode, const int sizeX, const int sizeY){
    // codeXへの代入
    for (int i = 0; i < sizeX; ++ i) {
        *(codeX + i) = *(originalCode + i);
    }
    
    // codeYへの代入
    for (int i = 0; i < sizeY; ++ i) {
        *(codeY + i) = *(originalCode + sizeX + i);
    }
}

// グレイコードからバイナリコードを生成
// binaryCode   : バイナリコードを入れる配列
// grayCode     : グレイコードが入っている配列
// bitNum       : ビット深度
void GeometricCalibration::grayCode2binaryCode(bool* const binaryCode, const bool* const grayCode, const int bitNum){
	//printCurrentPattern(grayCode, bitNum);
    // 最上位ビット
    *binaryCode = *grayCode;
    
    // それ以外のビット
    for (int i = 1; i < bitNum; ++ i) {
        *(binaryCode + i) = *(grayCode + i) ^ *(binaryCode + i - 1);
    }
}
// 上のテスト
void GeometricCalibration::test_grayCode2binaryCode(void){
    // init
    const int bitNum = 4;
    bool binary[bitNum] = {0,0,0,0};
    bool gray[bitNum] = {1,0,0,0};
    
    grayCode2binaryCode(binary, gray, bitNum);
    cout << "gray code" << endl;
    printCurrentPattern(gray, bitNum);
    cout << "binary code" << endl;
    printCurrentPattern(binary, bitNum);
}

// 二進数から十進数へ変換
int GeometricCalibration::binary2decimal(const bool* const binaryCode, const int depth){
    // init
    int binary = 0;
    for (int i = 0, timeNum = 1; i < depth; ++ i, timeNum *= 2) {
        binary += timeNum * (*(binaryCode + depth - 1 - i));
    }
    
    return binary;
}

void GeometricCalibration::test_grayCode2binaryCode_binary2decimal(void){
    // init
    const int bitNum = 2;
    bool binary[bitNum] = {0,0};
    bool gray[bitNum] = {0,0};
    grayCode2binaryCode(binary, gray, bitNum);
    cout << "gray code" << endl; printCurrentPattern(gray, bitNum);
    cout << "binary code" << endl; printCurrentPattern(binary, bitNum);
    int decimal = binary2decimal(binary, bitNum);
    _print(decimal);cout << endl;
    
    gray[1] = 1;
    grayCode2binaryCode(binary, gray, bitNum); cout << "gray code" << endl;
    printCurrentPattern(gray, bitNum); cout << "binary code" << endl;
    printCurrentPattern(binary, bitNum);
    decimal = binary2decimal(binary, bitNum);
    _print(decimal);cout << endl;
    
    gray[0] = 1;
    grayCode2binaryCode(binary, gray, bitNum); cout << "gray code" << endl;
    printCurrentPattern(gray, bitNum); cout << "binary code" << endl;
    printCurrentPattern(binary, bitNum);
    decimal = binary2decimal(binary, bitNum);
    _print(decimal);cout << endl;
    
    gray[1] = 0;
    grayCode2binaryCode(binary, gray, bitNum); cout << "gray code" << endl;
    printCurrentPattern(gray, bitNum); cout << "binary code" << endl;
    printCurrentPattern(binary, bitNum);
    decimal = binary2decimal(binary, bitNum);
    _print(decimal);
}

// グレイコードから座標値を取得（インバースグレイコード？）
int GeometricCalibration::getPositionFromGrayCode(const bool* const grayCode, const int depth){
    // バイナリコードを生成
    bool* binaryCode = (bool*)malloc(sizeof(bool) * depth); // 純二進コードを入れる配列
    memset(binaryCode, 0, sizeof(bool) * depth);
    grayCode2binaryCode(binaryCode, grayCode, depth);
    
    // バイナリコードから数字を生成
    int binaryNum = binary2decimal(binaryCode, depth);
    free(binaryCode);
    
    return binaryNum;
}
// 上のテスト
void GeometricCalibration::test_getPositionFromGrayCode(void){
	bool in3[8*3] = {
		0,0,0,
		0,0,1,
		0,1,1,
		0,1,0,
		1,1,0,
		1,1,1,
		1,0,1,
		1,0,0
	};
	int out3[8] = {0,0,0,0,0,0,0,0};
	for(int i = 0; i < 8; ++ i){
		out3[i] = getPositionFromGrayCode((in3 + 3 * i), 3);
		printCurrentPattern((in3 + 3 * i), 3);
		_print(out3[i]);
	}
    
    
	bool in4[16*4] = {
		0,0,0,0,
		0,0,0,1,
		0,0,1,1,
		0,0,1,0,
		0,1,1,0,
		0,1,1,1,
		0,1,0,1,
		0,1,0,0,
        
		1,1,0,0,
		1,1,0,1,
		1,1,1,1,
		1,1,1,0,
		1,0,1,0,
		1,0,1,1,
		1,0,0,1,
		1,0,0,0
	};
	int out4[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	for(int i = 0; i < 16; ++ i){
		out4[i] = getPositionFromGrayCode((in4 + 4 * i), 4);
		printCurrentPattern((in4 + 4 * i), 4);
		_print(out4[i]);
	}
}

// カメラからプロジェクタへのアクセスマップの設定を行う
// *c2pMap              : 代入するアクセスマップ
// *codeMapCamera       : カメラのコードマップ
// *codeMapProjector    : プロジェクタのコードマップ (いらない)
// *cameraSize          : カメラ画像の大きさ
// *projectorSize       : プロジェクタ画像の大きさ
// *depthSize           : X,Y座標のビット深度
void GeometricCalibration::setAccessMap(Point* const c2pMap, const bool* codeMapCamera, const bool* codeMapProjector, const Size* cameraSize, const Size* projectorSize, const Size* const depthSize){
    // カメラマップの全画素探索
    for (int cy = 0; cy < cameraSize->height; ++ cy) {
        for (int cx = 0; cx < cameraSize->width; ++ cx) {
            // init
            int cameraPos = cx + cy * cameraSize->width;    // カメラの位置
            bool *grayCodeX = (bool*)malloc(sizeof(bool) * depthSize->width);// グレイコードのX座標
            bool *grayCodeY = (bool*)malloc(sizeof(bool) * depthSize->height);// グレイコードのY座標
            memset(grayCodeX, 0, sizeof(bool) * depthSize->width);
            memset(grayCodeY, 0, sizeof(bool) * depthSize->height);
            
            // カメラの空間コードからxyそれぞれのコード値を分離 maybe ok
            divideCode(grayCodeX, grayCodeY, codeMapCamera + cameraPos * (depthSize->width + depthSize->height), depthSize->width, depthSize->height);
            
            // 座標値を代入 ok
            int px = getPositionFromGrayCode(grayCodeX, depthSize->width);
            int py = getPositionFromGrayCode(grayCodeY, depthSize->height);
            
            // error
            if(px >= projectorSize->width){
//                cout << "X value of projector is out of bound (border : " << projectorSize->width << ")" << endl;
//                ERROR_PRINT(px);
                px = py = 0;
                //return;
            }else if(py >= projectorSize->height){
//                cout << "Y value of projector is out of bound (border : " << projectorSize->height << ")" << endl;
//                ERROR_PRINT(py);
                px = py = 0;
                //return;
            }
            
            // set
            setPoint(c2pMap + cameraPos, px, py);
            
            // free
            free(grayCodeX);
            free(grayCodeY);
        }
    }
	cout << "---created access map---" << endl;
}

// 上のテスト
bool GeometricCalibration::test_setAccessMap(void){
    
    
    return true;
}

// カメラ座標からプロジェクタ座標を取得する
// projector    : 代入するプロジェクタ座標
// camera       : カメラ座標
// accessMapC2P : カメラ座標からプロジェクタ座標へ変換するマップ
// cameraWidth  : カメラの幅
void GeometricCalibration::getProjectorPoint(Point* const projector, const Point* const camera, const Point* const accessMapC2P, const int cameraWidth){
    const Point* accessMapPtr = accessMapC2P + camera->x + camera->y * cameraWidth;
    projector->x = accessMapPtr->x;
    projector->y = accessMapPtr->y;
}

// 幾何キャリブレーションのテスト
void GeometricCalibration::test_geometricCalibration(Point* const accessMapC2P, VideoCapture *video, const Size* const cameraSize, const Size* const projectorSize){
	// init
	Mat camera, projector;    // カメラ画像
	string gtCameraWindowName = "camera";       // カメラのウィンドウ名
	string gtProjectorWindowName = "projector"; // プロジェクタのウィンドウ名
    int windowSize = 2; // 点の大きさ
	Point cp(50, 50), pp(0, 0);   // カメラ・プロジェクタ画像の位置
    getProjectorPoint(&pp, &cp, accessMapC2P, cameraSize->width);
    
    // loop
	while(1){
        // カメラ画像の取得
//        Mat frame;
//		*video >> frame;
//        camera = frame.clone();
		*video >> camera;
        projector = Mat::zeros(*projectorSize, CV_8UC3);    // プロジェクタ画像
        
        // 3x3の塗りつぶし
        int ch = 3;
        for (int y = -windowSize; y <= windowSize; ++ y) {
            // カメラ・プロジェクタ画像へのポインタ
            uchar* const camPtr = camera.ptr(cp.y + y);
            uchar* const prjPtr = projector.ptr(pp.y + y);
            
            for (int x = -windowSize; x <= windowSize; ++ x) {
                int camX = max(cp.x + x, 0), prjX = max(pp.x + x, 0);   // カメラ・プロジェクタの座標
                
                // カメラ画像への代入
                camPtr[camX * ch + 0] = 0;
                camPtr[camX * ch + 1] = 0;
                camPtr[camX * ch + 2] = UCHAR_MAX;
                // プロジェクタ画像の代入
                ////////////// EXC_BAC_ACCESS
                prjPtr[prjX * ch + 0] = UCHAR_MAX;
                //////////////
                prjPtr[prjX * ch + 1] = 0;
                prjPtr[prjX * ch + 2] = UCHAR_MAX;
            }
        }
        
        // ウィンドウを表示
		imshow(gtCameraWindowName, camera); // camera image
		cvMoveWindow(gtCameraWindowName.c_str(), projectorSize->width, 0);
		imshow(gtProjectorWindowName, projector); // projector image
		waitKey(1);
        
        // advanced camera pointer
        const int step = 50;
        cp.x += step;
        if (cp.x + windowSize >= cameraSize->width) {
            cp.x = windowSize;
            cp.y += step;
            
            if (cp.y + windowSize >= cameraSize->height) {
                cp.y = windowSize;
            }
        }
        getProjectorPoint(&pp, &cp, accessMapC2P, cameraSize->width);
        _print2(cp, pp);
	}
}

// 多分ミスってる
void GeometricCalibration::test_accessMap(const Point* const accessMapCam2Pro, const Size& cameraSize, const Size& projectorSize, const char* _fileName){
    Mat accessImage = Mat::zeros(cameraSize, CV_8UC3); // アクセスマップ画像
    accessMap2image(&accessImage, accessMapCam2Pro, cameraSize, projectorSize);
    imshow(_fileName, accessImage);
}

// 
void GeometricCalibration::convertArrPt2MatVec(cv::Mat_<cv::Vec2i>* const dst, const cv::Point_<int>* src, const cv::Size& _srcSize){
    //
    int cols = dst->cols, rows = dst->rows;
    const Size dstSize(cols, rows);
    if (dstSize != _srcSize) {
        _print3("size is different", dstSize, _srcSize);
        exit(-1);
    }
    
    //
    if (dst->isContinuous()) {
        cols *= rows;
        rows = 1;
    }
    for (int y = 0, i = 0; y < rows; ++ y) {
        Vec2i* p_dst = dst->ptr<Vec2i>(y);
        for (int x = 0; x < cols; ++ x, ++ i) {
            p_dst[x] = Vec2i(src[i]);
        }
    }
}

// 幾何キャリブレーション
bool GeometricCalibration::doCalibration(Mat_<Vec2i>* const _accessMapCam2Pro, cv::VideoCapture* video){
    // init camera
    ProCam* l_procam = getProCam();
    Size* cameraSize = l_procam->getCameraSize();    // カメラの大きさ

    // init projection image
    Size projectionSize(PRJ_SIZE_WIDTH, PRJ_SIZE_HEIGHT);       // 投影サイズ
    Mat projectionImage = cv::Mat::zeros(projectionSize, CV_8UC3);
    Size layerSize(calcBitCodeNumber(projectionSize.width), calcBitCodeNumber(projectionSize.height));
    
    const int accessBitNum = layerSize.width + layerSize.height;  // アクセスに必要なビット数
    const int pixelNumProjector = projectionSize.width * projectionSize.height;        // 全画素数
    const int pixelNumCamera = cameraSize->width * cameraSize->height;        // 全画素数
    
    // 空間コード画像（大きさ：(縦層+横層)*全画素数）
    bool *grayCodeMapProjector = (bool*)malloc(sizeof(bool) * accessBitNum * pixelNumProjector);// プロジェクタのグレイコードマップ
    bool *grayCodeMapCamera = (bool*)malloc(sizeof(bool) * accessBitNum * pixelNumCamera);      // カメラのグレイコードマップ
    memset(grayCodeMapProjector, 0, sizeof(bool) * accessBitNum * pixelNumProjector);
    memset(grayCodeMapCamera, 0, sizeof(bool) * accessBitNum * pixelNumCamera);
    
    // 縦横の縞模様を投影しグレイコードをプロジェクタ，カメラ双方に付与する
    int offset = 0; // 初期ビットの位置
    for (int timeStep = 1; timeStep <= layerSize.width; ++ timeStep, ++ offset) {
        addSpatialCodeOfProCam(grayCodeMapProjector, grayCodeMapCamera, &projectionSize, cameraSize, timeStep, offset, Vertical, video);
    }
    for (int timeStep = 1; timeStep <= layerSize.height; ++ timeStep, ++ offset) {
        addSpatialCodeOfProCam(grayCodeMapProjector, grayCodeMapCamera, &projectionSize, cameraSize, timeStep, offset, Horizon, video);
    }
    
	// 使用したウィンドウの削除
	destroyWindow(W_NAME_GEO_CAMERA);
	destroyWindow(W_NAME_GEO_PROJECTOR);
    
    // プロカム間のアクセスマップを作る
    Point* l_accessMapCam2Pro = new Point[cameraSize->area()];  // Mat_<Vec2i>へ変換する為の変数
//    setAccessMap(_accessMapCam2Pro, grayCodeMapCamera, grayCodeMapProjector, cameraSize, &projectionSize, &layerSize);
    setAccessMap(l_accessMapCam2Pro, grayCodeMapCamera, grayCodeMapProjector, cameraSize, &projectionSize, &layerSize);
    
    // 空間コード画像の解放
	free(grayCodeMapProjector);
	free(grayCodeMapCamera);
    
    // アクセスマップのテスト
    test_accessMap(l_accessMapCam2Pro, *cameraSize, projectionSize, "access map image");
    
	// 幾何キャリブレーションのテスト
//	test_geometricCalibration(_accessMapCam2Pro, &camera, &cameraSize, &projectionSize);
    
    // Point[] -> Mat_<Vec2i>
    convertArrPt2MatVec(_accessMapCam2Pro, l_accessMapCam2Pro, *cameraSize);
    
    delete [] l_accessMapCam2Pro;
    return true;
}

