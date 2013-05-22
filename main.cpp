//
//  main.cpp
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/03/11.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//

#include <iostream>
#include "common.h"
#include "myOpenCV.h"
//#include "myOpenGL.h"

using namespace std;
using namespace cv;


// 現在のパターンの表示
void printCurrentPattern(const bool* const pattern, const int patternSize){
    for(int i = 0; i < patternSize; ++ i){
        cout << *(pattern + i);
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
void image2map(bool* const map, Mat* const image, const Size* const mapSize){
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

// パターンマップから画像を生成する
// image    : 生成する画像
// map      : 参照するパターンマップ
// mapSize  : 上二つの大きさ
/*void accessMap2image(Mat *image, const Point* const map, const Size* const mapSize){
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
}*/

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
    
    // init gray code image
    makeGrayCodePattern(posiPattern, patternSize, layerNum);
    
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
// 上のテスト
void test_insertAccessMap(void){
    
}

// 入力された投影像を投影・撮影を行い，撮影像を出力する
void captureProjectionImage(Mat* const captureImage, const Mat* const projectionImage, VideoCapture *videoStream){
    // 投影
    imshow(W_NAME_GEO_PROJECTOR, *projectionImage);  // posi pattern
    waitKey(SLEEP_TIME);
    
    // 撮影
    Mat image;
    *videoStream >> image;
	waitKey(SLEEP_TIME);
    *videoStream >> image;
    *captureImage = image.clone();
    //imshow(W_NAME_GEO_CAMERA, *captureImage);
	//cvMoveWindow(W_NAME_GEO_CAMERA, projectionImage->rows, 0);
	waitKey(SLEEP_TIME);
}

int num = 0;
// プロカムの空間コードを追加していく
// spatialCodeProjector : プロジェクタの空間コード格納ポインタ
// spatialCodeCamera    : カメラの空間コード格納ポインタ
// patternLayerNum  : 投影パターンの層番号
// offsetBit        : 追加するビットの位置
// direction        : 縞の方向
// camera           : カメラのストリーム
void addSpatialCodeOfProCam(bool* const spatialCodeProjector, bool* const spatialCodeCamera, const Size* const projectorSize, const Size* const cameraSize, const int patternLayerNum, const int offset, const stripeDirection direction, VideoCapture *videoStream){
    // init
    Mat projectionPosiImage = cv::Mat::zeros(*projectorSize, CV_8UC3);  // ポジ画像
    
    // バイナリマップを作成
    bool *binaryMapProjector = (bool*)malloc(sizeof(bool) * projectorSize->width * projectorSize->height);      // プロジェクタのアクセスマップ
    bool *binaryMapCamera = (bool*)malloc(sizeof(bool) * cameraSize->width * cameraSize->height);      // カメラのアクセスマップ
    createBinaryMap(binaryMapProjector, projectorSize, patternLayerNum, direction);
    
    // バイナリマップから画像を生成
    map2image(&projectionPosiImage, binaryMapProjector, projectorSize);
    
    // projection and shot posi image
    // posi
    Mat posiImage, negaImage;
    waitKey(SLEEP_TIME);
    captureProjectionImage(&posiImage, &projectionPosiImage, videoStream);
    // nega
    Mat projectionNegaImage = ~projectionPosiImage;     // ネガ画像
    captureProjectionImage(&negaImage, &projectionNegaImage, videoStream);
    
    // カラー画像からグレー画像へ変換
    cvtColor(posiImage, posiImage, CV_BGR2GRAY);
    cvtColor(negaImage, negaImage, CV_BGR2GRAY);
	
    // ポジネガ撮影像を符号有り１６ビットに拡張 ok
	Mat posiImage16bit, negaImage16bit;
    posiImage.convertTo(posiImage16bit, CV_16SC1);
    negaImage.convertTo(negaImage16bit, CV_16SC1);
    
    // ポジネガ撮影像の差分 ok
    Mat diffPosiNega16s = posiImage16bit - negaImage16bit;

    // 差分画像の二値化
    image2map(binaryMapCamera, &diffPosiNega16s, cameraSize);
    map2image(&diffPosiNega16s, binaryMapCamera, cameraSize);
    
    // 差分画像の表示 ok
    //imshow16s("diff image", &diffPosiNega16s);
    //cvMoveWindow("diff image", projectorSize->width, 0);
    
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
void divideCode(bool* const codeX, bool* const codeY, const bool* const originalCode, const int sizeX, const int sizeY){
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
void grayCode2binaryCode(bool* const binaryCode, const bool* const grayCode, const int bitNum){
	//printCurrentPattern(grayCode, bitNum);
    // 最上位ビット
    *binaryCode = *grayCode;
    
    // それ以外のビット
    for (int i = 1; i < bitNum; ++ i) {
        *(binaryCode + i) = *(grayCode + i) ^ *(binaryCode + i - 1);
    }
}
// 上のテスト
void test_grayCode2binaryCode(void){
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
int binary2decimal(const bool* const binaryCode, const int depth){
    // init
    int binary = 0;
    for (int i = 0, timeNum = 1; i < depth; ++ i, timeNum *= 2) {
        binary += timeNum * (*(binaryCode + depth - 1 - i));
    }
    
    return binary;
}

void test_grayCode2binaryCode_binary2decimal(void){
    // init
    const int bitNum = 2;
    bool binary[bitNum] = {0,0};
    bool gray[bitNum] = {0,0};
    grayCode2binaryCode(binary, gray, bitNum); cout << "gray code" << endl;
    printCurrentPattern(gray, bitNum); cout << "binary code" << endl;
    printCurrentPattern(binary, bitNum);
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
int getPositionFromGrayCode(const bool* const grayCode, const int depth){
    // バイナリコードを生成
    bool* binaryCode = (bool*)malloc(sizeof(bool) * depth); // 純二進コードを入れる配列
    memset(binaryCode, 0, sizeof(bool) * depth);
    grayCode2binaryCode(binaryCode, grayCode, depth);
    
    // バイナリコードから数字を生成
    int binaryNum = binary2decimal(binaryCode, depth);
    
    return binaryNum;
}
// 上のテスト
void test_getPositionFromGrayCode(void){
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
void setAccessMap(Point* const c2pMap, const bool* codeMapCamera, const bool* codeMapProjector, const Size* cameraSize, const Size* projectorSize, const Size* const depthSize){
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
            divideCode(grayCodeX, grayCodeY, codeMapCamera + cameraPos, depthSize->width, depthSize->height);
            
            // 座標値を代入 ok
            int px = getPositionFromGrayCode(grayCodeX, depthSize->width);
            int py = getPositionFromGrayCode(grayCodeY, depthSize->height);
            
            // error
            if(px >= projectorSize->width){
                cout << "X value of projector is out of bound (border : " << projectorSize->width << ")" << endl;
                ERROR_PRINT(px);
                return;
            }else if(py >= projectorSize->height){
                cout << "Y value of projector is out of bound (border : " << projectorSize->height << ")" << endl;
                ERROR_PRINT(py);
                return;
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

// カメラ座標からプロジェクタ座標を取得する
// projector    : 代入するプロジェクタ座標
// camera       : カメラ座標
// accessMapC2P : カメラ座標からプロジェクタ座標へ変換するマップ
// cameraWidth  : カメラの幅
void getProjectorPoint(Point* const projector, const Point* const camera, const Point* const accessMapC2P, const int cameraWidth){
    const Point* accessMapPtr = accessMapC2P + camera->x + camera->y * cameraWidth;
    projector->x = accessMapPtr->x;
    projector->y = accessMapPtr->y;
}

// 幾何キャリブレーションのテスト
void test_geometricCalibration(Point* const accessMapC2P, VideoCapture *video, const Size* const cameraSize, const Size* const projectorSize){
	// init
	Mat camera, projector;    // カメラ画像
	string gtCameraWindowName = "camera";       // カメラのウィンドウ名
	string gtProjectorWindowName = "projector"; // プロジェクタのウィンドウ名
    int windowSize = 2; // 点の大きさ
	Point cp(windowSize, windowSize), pp(0, 0);   // カメラ・プロジェクタ画像の位置
    getProjectorPoint(&pp, &cp, accessMapC2P, cameraSize->width);
    
    // loop
	while(1){
        // カメラ画像の取得(浅いコピー)
        Mat frame;
		*video >> frame;
        camera = frame.clone();
        projector = Mat::zeros(*projectorSize, CV_8UC3);    // プロジェクタ画像
        
        // 3x3の塗りつぶし
        int ch = 3;
        for (int y = -windowSize; y <= windowSize; ++ y) {
            // カメラ・プロジェクタ画像へのポインタ
            uchar* const camPtr = camera.ptr(cp.y + y);
            uchar* const prjPtr = projector.ptr(pp.y + y);
            
            for (int x = -windowSize; x <= windowSize; ++ x) {
                int camX = cp.x + x, prjX = pp.x + x;
                // カメラ画像への代入
                camPtr[camX * ch + 0] = 0;
                camPtr[camX * ch + 1] = 0;
                camPtr[camX * ch + 2] = UCHAR_MAX;
                // プロジェクタ画像の代入
                prjPtr[prjX * ch + 0] = UCHAR_MAX;
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

// main method
int main(int argc, const char * argv[])
{
    //test_convertMatDepth16sTo8u();
    //test_convertNumber16sTo8u();
    test_grayCode2binaryCode_binary2decimal();
    
    // init camera
    VideoCapture camera(0);
    if( !camera.isOpened() ){
        std::cerr << "ERROR : camera is not opened !!" << std::endl;
        return 1;
    }
    Mat frame;                                  // カメラ画像
    camera >> frame;
    Size cameraSize(frame.cols, frame.rows);    // カメラの大きさ
    cout << "cameraSize = " << cameraSize << endl;
    //imshow("camera image", frame);
    
    // init projection image
    Size projectionSize(PRJ_SIZE_WIDTH, PRJ_SIZE_HEIGHT);       // 投影サイズ
    Mat projectionImage = cv::Mat::zeros(projectionSize, CV_8UC3);
    Size layerSize(calcBitCodeNumber(projectionSize.width), calcBitCodeNumber(projectionSize.height));
    const int accessBitNum = layerSize.width + layerSize.height;  // アクセスに必要なビット数
    const int pixelNumProjector = projectionSize.width * projectionSize.height;        // 全画素数
    const int pixelNumCamera = cameraSize.width * cameraSize.height;        // 全画素数
    
    // 空間コード画像（大きさ：(縦層+横層)*全画素数）
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
    
	// 幾何キャリブレーションに使用したウィンドウの削除
	destroyWindow(W_NAME_GEO_CAMERA);
	destroyWindow(W_NAME_GEO_PROJECTOR);
    
    // プロカム間のアクセスマップを作る
    Point *accessMapCam2Pro = (Point*)malloc(sizeof(Point) * pixelNumCamera);// カメラからプロジェクタへのアクセスマップ
    initPoint(accessMapCam2Pro, pixelNumCamera);
    setAccessMap(accessMapCam2Pro, grayCodeMapCamera, grayCodeMapProjector, &cameraSize, &projectionSize, &layerSize);
    
    // 空間コード画像の解放
	free(grayCodeMapProjector);
	free(grayCodeMapCamera);
    
	// 幾何キャリブレーションのテスト
	test_geometricCalibration(accessMapCam2Pro, &camera, &cameraSize, &projectionSize);
    
	// アクセスマップの解放
	free(accessMapCam2Pro);
    
    //myInitGlut(argc, argv);
    /*
     // gl cv test
     camera >> frame;
     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame.size().width, frame.size().height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, frame.data);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);*/
    
    
    return 0;
}

