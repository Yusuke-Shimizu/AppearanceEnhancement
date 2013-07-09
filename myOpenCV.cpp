//
//  myOpenCV.cpp
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/05/10.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//

#include <cmath>
#include "myOpenCV.h"
#include "common.h"

using namespace cv;

///////////////////////////////  check method ///////////////////////////////
// Matの大きさのチェック
// return   : 大きさが同じならtrue，違うならfalse
bool isEqualSize(const cv::Mat& m1, const cv::Mat& m2){
    if (m1.size() == m2.size()) {
        return true;
    } else {
        return false;
    }
}
bool isEqualSize(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3){
    return isEqualSize(m1, m2) & isEqualSize(m2, m3);
}
bool isEqualSize(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4){
    return isEqualSize(m1, m2, m3) & isEqualSize(m3, m4);
}
bool isEqualSize(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4, const cv::Mat& m5){
    return isEqualSize(m1, m2, m3, m4) & isEqualSize(m4, m5);
}
bool isEqualSize(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4, const cv::Mat& m5, const cv::Mat& m6){
    return isEqualSize(m1, m2, m3, m4, m5) & isEqualSize(m5, m6);
}
bool isDifferentSize(const cv::Mat& m1, const cv::Mat& m2) { return !isEqualSize(m1, m2); }
bool isDifferentSize(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3) { return !isEqualSize(m1, m2, m3); }
bool isDifferentSize(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4) { return !isEqualSize(m1, m2, m3, m4); }
bool isDifferentSize(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4, const cv::Mat& m5) { return !isEqualSize(m1, m2, m3, m4, m5); }
bool isDifferentSize(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4, const cv::Mat& m5, const cv::Mat& m6) { return !isEqualSize(m1, m2, m3, m4, m5, m6); }


// 複数のMatの連続性の確認
bool isContinuous(const cv::Mat& m1, const cv::Mat& m2){
    return m1.isContinuous() & m2.isContinuous();
}
bool isContinuous(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3){
    return isContinuous(m1, m2) & m3.isContinuous();
}
bool isContinuous(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4){
    return isContinuous(m1, m2, m3) & m4.isContinuous();
}
bool isContinuous(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4, const cv::Mat& m5){
    return isContinuous(m1, m2, m3, m4) & m5.isContinuous();
}
bool isContinuous(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4, const cv::Mat& m5, const cv::Mat& m6){
    return isContinuous(m1, m2, m3, m4, m5) & m6.isContinuous();
}


// ビット深度が等しいかどうか
bool isEqualDepth(const cv::Mat& m1, const cv::Mat& m2){
    if (m1.depth() == m2.depth()) {
        return true;
    } else {
        return false;
    }
}

// チャンネル数が等しいかどうか
bool isEqualChannel(const cv::Mat& m1, const cv::Mat& m2){
    if (m1.channels() == m2.channels()) {
        return true;
    } else {
        return false;
    }
}

// タイプ(ex.CV_64FC1)が同じかどうかの判定
bool isEqualType(const cv::Mat& m1, const cv::Mat& m2){
    return isEqualChannel(m1, m2) & isEqualChannel(m1, m2);
}
bool isEqualType(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3){
    return isEqualType(m1, m2) & isEqualType(m2, m3);
}
bool isEqualType(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4){
    return isEqualType(m1, m2, m3) & isEqualType(m3, m4);
}
bool isEqualType(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4, const cv::Mat& m5){
    return isEqualType(m1, m2, m3, m4) & isEqualType(m4, m5);
}
bool isEqualType(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4, const cv::Mat& m5, const cv::Mat& m6){
    return isEqualType(m1, m2, m3, m4, m5) & isEqualType(m5, m6);
}

// サイズとタイプが等しいかどうか
bool isEqualSizeAndType(const cv::Mat& m1, const cv::Mat& m2){
    return isEqualSize(m1, m2) & isEqualType(m1, m2);
}
bool isEqualSizeAndType(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3){
    return isEqualSizeAndType(m1, m2) & isEqualSizeAndType(m2, m3);
}
bool isEqualSizeAndType(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4){
    return isEqualSizeAndType(m1, m2, m3) & isEqualSizeAndType(m3, m4);
}
bool isEqualSizeAndType(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4, const cv::Mat& m5){
    return isEqualSizeAndType(m1, m2, m3, m4) & isEqualSizeAndType(m4, m5);
}
bool isEqualSizeAndType(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4, const cv::Mat& m5, const cv::Mat& m6){
    return isEqualSizeAndType(m1, m2, m3, m4, m5) & isEqualSizeAndType(m5, m6);
}


// Point配列が全て正しいかどうか判定
bool isEqualPoint(const cv::Point* const p1, const cv::Point* const p2, const int length){
    for (int i = 0; i < length; ++ i) {
        if (*(p1 + i) != *(p2 + i)) {
            return false;
        }
    }
    
    return true;
}

///////////////////////////////  set method ///////////////////////////////
// 色のセッティング
// it -> (red, green, blue)
void setColor(const MatIterator_<Vec3b> it, const uchar red, const uchar green, const uchar blue){
    (*it)[0] = blue;
    (*it)[1] = green;
    (*it)[2] = red;
}
void setColor(const MatIterator_<Vec3b> it, const uchar luminance){
    setColor(it, luminance, luminance, luminance);
}
void setColor(cv::Mat* const _mat, const double& _red, const double& _green, const double& _blue){
    _mat->at<double>(0, 0) = _blue;
    _mat->at<double>(0, 1) = _green;
    _mat->at<double>(0, 2) = _red;
}
void setColor(cv::Mat* const _mat, const double& _luminance){
    setColor(_mat, _luminance, _luminance, _luminance);
}
bool setColor(cv::Mat* const _dst, const cv::Mat& _src){
    // check size
    Mat color = Mat::zeros(3, 1, CV_8SC1);
    if ( !isEqualSize(*_dst, _src, color) ){
        std::cerr << "mat size is different" << std::endl;
        ERROR_PRINT3(*_dst, _src, color);
        return false;
    }
    color.release();
    
    // setting
    setColor(_dst, _src.at<double>(0, 0), _src.at<double>(0, 1), _src.at<double>(0, 2));
    
    return true;
}

bool setVecColor(cv::Vec3b* const _color, const char _luminance, const ColorName _cName){
    initVec3b(_color);
    (*_color)[_cName] = _luminance;
    return true;
}



// Pointのセッティング
void setPoint(cv::Point* const _p, const int _x, const int _y){
    _p->x = _x;
    _p->y = _y;
}

///////////////////////////////  get method ///////////////////////////////
// Pointの値をx, yに代入
// output / _x  : x座標を入れる値
// output / _y  : y座標を入れる値
// input / _p  : x座標を入れる値
bool getPoint(int* const _x, int* const _y, const cv::Point& _p){
    *_x = _p.x;
    *_y = _p.y;
    return true;
}

// 画像のある座標の値を取得する
// input / _image   : 参照画像
// input / _point   : 参照座標
// return           : 画素の値
uchar getPixelNumuc(const cv::Mat& _image, const cv::Point& _point, const ColorName _cName){
    uchar pixNum = 0;
    const int ch = _image.channels();
    const uchar *pImage = _image.ptr<uchar>(_point.y);
    pixNum = pImage[_point.x * ch + _cName];
    
    return pixNum;
}
double getPixelNumd(const cv::Mat& _image, const cv::Point& _point, const ColorName _cName){
    double pixNum = 0;
    const int ch = _image.channels();
    const double *pImage = _image.ptr<double>(_point.y);
    pixNum = pImage[_point.x * ch + _cName];
    
    return pixNum;
}
const cv::Vec3d* getPixelNumd(const cv::Mat& _image, const cv::Point& _point){
    const cv::Vec3d *pImage = _image.ptr<cv::Vec3d>(_point.y);
    
    return &pImage[_point.x];
}

///////////////////////////////  print method ///////////////////////////////
// Matの様々な要素を表示
void printMatPropaty(const Mat& m1){
    std::cout << "--------------------------"  << std::endl;
    // 行数
    std::cout << "rows:" << m1.rows <<std::endl;
    // 列数
    std::cout << "cols:" << m1.cols << std::endl;
    // 次元数
    std::cout << "dims:" << m1.dims << std::endl;
    // サイズ（2次元の場合）
    std::cout << "size[]:" << m1.size().width << "," << m1.size().height << std::endl;
    // ビット深度ID
    std::cout << "depth (ID):" << m1.depth() << "(=" << CV_64F << ")" << std::endl;
    // チャンネル数
    std::cout << "channels:" << m1.channels() << std::endl;
    // （複数チャンネルから成る）1要素のサイズ [バイト単位]
    std::cout << "elemSize:" << m1.elemSize() << "[byte]" << std::endl;
    // 1要素内の1チャンネル分のサイズ [バイト単位]
    std::cout << "elemSize1 (elemSize/channels):" << m1.elemSize1() << "[byte]" << std::endl;
    // 要素の総数
    std::cout << "total:" << m1.total() << std::endl;
    // ステップ数 [バイト単位]
    std::cout << "step:" << m1.step << "[byte]" << std::endl;
    // 1ステップ内のチャンネル総数
    std::cout << "step1 (step/elemSize1):" << m1.step1()  << std::endl;
    // データは連続か？
    std::cout << "isContinuous:" << (m1.isContinuous()?"true":"false") << std::endl;
    // 部分行列か？
    std::cout << "isSubmatrix:" << (m1.isSubmatrix()?"true":"false") << std::endl;
    // データは空か？
    std::cout << "empty:" << (m1.empty()?"true":"false") << std::endl;
    _print_bar;
}

// OpenCVのバージョン表示
void printOpenCVVersion(void) {
    std::cout << "version: " << CV_VERSION << std::endl;
    std::cout << "  major: " << CV_MAJOR_VERSION << std::endl;
    std::cout << "  minor: " << CV_MINOR_VERSION << std::endl;
    std::cout << "  subminor: " << CV_SUBMINOR_VERSION << std::endl;
    std::cout << "OpenCV >= 2.0.0: " << (OPENCV_VERSION_CODE>=OPENCV_VERSION(2,0,0)?"true":"false") << std::endl;
}

///////////////////////////////  init method ///////////////////////////////
// Pointの初期化
void initPoint(cv::Point* const p, const int size){
    for (int i = 0; i < size; ++ i) {
        (p + i)->x = 0;
        (p + i)->y = 0;
    }
}

// Matの初期化
// output / _aMat : 初期化する行列の配列
// input / _size : Matの大きさ
void initMat(cv::Mat* const _aMat, const int _size){
    for (int i = 0; i < _size; ++ i) {
//        *(_aMat + i) = Mat::zeros(3, 3, CV_64FC1);
    }
}

// cv::Vec3bの初期化
void initVec3b(cv::Vec3b* const _vector){
    for (int i = 0; i < 3; ++ i) {
        (*_vector)[i] = 0;
    }
}

///////////////////////////////  convert method ///////////////////////////////
// matからopenGL型に変換
void mat2char(unsigned char c[], const Mat *m){
	_print(m->rows);
	_print(m->cols);
	_print(m->dims);
	int channel = m->channels();
	_print(channel);
    
	for(int r = 0, i = 0; r < m->rows; ++ r){
		for(int c = 0; c < m->cols; ++ c){
			for(int ch = 0; ch < m->channels(); ++ ch, ++ i){
				//*(c + i) =
				//	m->at<int>(r, c);
			}
		}
	}
}

// ある範囲の数からある範囲の数へ変換
// キャストで四捨五入されてなく，全て切り捨てされてる
uchar convertNumber16sTo8u(const short src, const Size* charRange, const Size* intRange){
    double A = intRange->width, B = intRange->height, a = charRange->width, b = charRange->height;
    double tmp = (A-B)/(a-b)*src + (a*B-b*A)/(a-b);
    return (uchar)tmp;
}
// 上のテスト
void test_convertNumber16sTo8u(void){
    for (int i = 0; i < 100; ++ i) {
        const Size smallRange(-40, 10 + i), bigRange(0, 25);
        const short src = 2;
        unsigned char tmp = convertNumber16sTo8u(src, &smallRange, &bigRange);
        _print2(src, (int)tmp);std::cout << std::endl;
    }
}

// Matの符号有り16bitから符号無し8bitへ変換
// dst8u    : 変換後の符号無し８ビット行列
// src16s   : 変換前の符号有り１６ビット行列
void convertMatDepth16sTo8u(cv::Mat* const dst8u, const cv::Mat* const src16s){
    // error processing
    if (dst8u->depth() != CV_8UC1) {            // ビット深度の確認
        int depth_dst8u = dst8u->depth();
        ERROR_PRINT(depth_dst8u);
        return;
    } else if (src16s->depth() != CV_16SC1) {
        int depth_src16s = src16s->depth();
        ERROR_PRINT(depth_src16s);
        return;
    } else if (src16s->rows != dst8u->rows) {   // 大きさの一致確認
        ERROR_PRINT(src16s->rows);
        ERROR_PRINT(dst8u->rows);
        return;
    } else if (src16s->cols != dst8u->cols) {
        ERROR_PRINT(src16s->cols);
        ERROR_PRINT(dst8u->cols);
        return;
    }
    
    // ucharとintの大きさ
    const Size charRange(DEPTH_U8BIT_MIN, DEPTH_U8BIT_MAX), intRange(DEPTH_S16BIT_MIN, DEPTH_S16BIT_MAX);
    int rows = src16s->rows, cols = src16s->cols;   // 行と列の大きさ
    
    // 連続性の確認
    if ( isContinuous(*src16s, *dst8u) ) {
        // 連続ならばループを二重から一重に変更
        cols *= rows;
        rows = 1;
    }
    
    // 行列へアクセスし個々に変換
    for (int y = 0; y < rows; ++ y) {
        // init pointer
        const short *src = src16s->ptr<short>(y);
        uchar *dst = dst8u->ptr<uchar>(y);
        
        for (int x = 0; x < cols; ++ x) {
            dst[x] = convertNumber16sTo8u(src[x], &charRange, &intRange);
        }
    }
}
// 上のテスト
void test_convertMatDepth16sTo8u(void){
    // init
    Mat image16s = Mat::zeros(10, 10, CV_16SC1);
	Mat image8u = Mat::zeros(image16s.rows, image16s.cols, CV_8UC1);
    MatIterator_<int> im16_itr = image16s.begin<int>();
    double A = DEPTH_S16BIT_MIN, B = DEPTH_S16BIT_MAX, a = DEPTH_U8BIT_MIN, b = DEPTH_U8BIT_MAX;
    _print4(a, b, A, B);
    
    for (int i = A; im16_itr != image16s.end<int>(); ++ im16_itr, i += 257 * 17) {
        *im16_itr = i;
        
        // back to zero
        if (i > B) {
            i = A;
        }
    }
    
    convertMatDepth16sTo8u(&image8u, &image16s);
    _print(image16s);
    _print(image8u);
}

// BGR形式からRGB形式に変換する
// output / _rgb    : 出力するRGB形式のベクタ
// input / _bgr     : 変換したいBGR形式のベクタ
bool convertBGRtoRGB(cv::Vec3b* const _rgb, const cv::Vec3b& _bgr){
    Vec3b tmp(_bgr[CV_RED], _bgr[CV_GREEN], _bgr[CV_BLUE]);
    *_rgb = tmp;
    
    return true;
}

// Vec9d -> Mat(3x3)
bool convertVecToMat(cv::Mat_<double>* const _dst, const Vec9d& _src){
    // error processing
    Mat m3x3(3, 3, CV_64FC1);
    if ( !isEqualSizeAndType(*_dst, m3x3) ) {
        ERROR_PRINT2("this matrix is not 3x3 matrix", *_dst);
        return false;
    }
    
    // setting
    int rows = _dst->rows, cols = _dst->cols;
    for (int y = 0, i = 0; y < rows; ++ y) {
        double* p_dst = _dst->ptr<double>(y);
        for (int x = 0; x < cols; ++ x, ++ i) {
            p_dst[x] = _src[i];
            _print2(p_dst[x], _src[i]);
        }
    }
    return true;
}

// Mat(3x3) -> Vec9d
bool convertMatToVec(Vec9d* const _dst, const cv::Mat_<double>& _src){
    // error processing
    Mat m3x3(3, 3, CV_64FC1);
    if ( !isEqualSizeAndType(_src, m3x3) ) {
        ERROR_PRINT2("this matrix is not 3x3 matrix", _src);
        return false;
    }

    // setting
    Vec9d tmp(0,0,0,0,0,0,0,0,0);
    int rows = _src.rows, cols = _src.cols;
    for (int y = 0, i = 0; y < rows; ++ y) {
        const double* p_src = _src.ptr<double>(y);
        for (int x = 0; x < cols; ++ x, ++ i) {
            tmp[i] = p_src[x];
        }
    }
    *_dst = tmp;
    return true;
}
///////////////////////////////  other method ///////////////////////////////

// この関数はいずれmyMath.hppに移動させる！！！！！！！
bool roundXtoY(double* const _num, const double& X, const double& Y){
    *_num = std::max(*_num, X);
    *_num = std::min(*_num, Y);
    return true;
}
// 引数の値を０から１に丸める
// ex) round0to1(5) = 1, round0to1(0.1) = 0.1, round0to1(-0.2) = 0
bool round0to1(double* const _num) {
    return roundXtoY(_num, 0.0, 1.0);
}

// Matの値をXからYに丸める
bool roundXtoYForMat(cv::Mat* const _mat, const cv::Mat& _X, const cv::Mat& _Y) {
    // error processing
    if ( !isEqualSize(*_mat, _X, _Y) ) {
        std::cout << "mat size is different" << std::endl;
        ERROR_PRINT3(*_mat, _X, _Y);
        return false;
    }
    
    // 連続性の確認
    int cols = _mat->cols, rows = _mat->rows;
    if ( isContinuous(*_mat, _X, _Y) ) {
        // 連続ならばループを二重から一重に変更
        cols *= rows;
        rows = 1;
    }
    
    // 行列へアクセスし個々に変換
    for (int y = 0; y < rows; ++ y) {
        // init pointer
        double *p_dst = _mat->ptr<double>(y);
        const double *p_X = _X.ptr<double>(y);
        const double *p_Y = _Y.ptr<double>(y);
        
        for (int x = 0; x < cols; ++ x) {
            roundXtoY(&p_dst[x], *p_X, *p_Y);
        }
    }
    
    return true;
}

// 上の範囲が行列でなく値である場合
bool roundXtoYForMat(cv::Mat* const _mat, const double& _X, const double& _Y){
    const int cols = _mat->cols, rows = _mat->rows;
    Mat l_matX = Mat::ones(rows, cols, CV_64FC1) * _X;
    Mat l_matY = Mat::ones(rows, cols, CV_64FC1) * _Y;
    return roundXtoYForMat(_mat, l_matX, l_matY);
}

// Matの値を０から１に丸める
bool round0to1ForMat(cv::Mat* const _mat){
    return roundXtoYForMat(_mat, 0, 1);
}
//
void test_round0to1ForMat(void){
    Mat test = (Mat_<double>(3, 3) << -0.5, -0.1, 0, 0.1, 0.4, 0.99, 1, 1.9, 100);
    _print(test);
    round0to1ForMat(&test);
    _print(test);
}

///////////////////////////////  other method ///////////////////////////////
// 符号付き１６ビット行列を表示
// windowName   : ウィンドウ名
// mat16s       : 表示する画像（符号付き１６ビット）
void imshow16s(const char* const windowName, const Mat* const mat16s){
    //error processing
    if (mat16s->depth() != CV_16SC1) {
        int mat16sDepth = mat16s->depth();
        ERROR_PRINT(mat16sDepth);
        return;
    }
    
    // １６ビットから８ビットへ変換
    Mat diffPosiNega8u = Mat::zeros(mat16s->rows, mat16s->cols, CV_8UC1);
    convertMatDepth16sTo8u(&diffPosiNega8u, mat16s);
    
    // ８ビット画像を表示
    imshow(windowName, diffPosiNega8u);
}

// VideoCaptureのテスト
// カメラ使う度にVideoCaptureを生成すると，削除する時に時間食うのでやめたほうがいい
void videoCapture_test(void){
    while(1){
        VideoCapture video(0);
        if( !video.isOpened() ){
            std::cerr << "ERROR : camera is not opened !!" << std::endl;
            return;
        }
        Mat image;
        video >> image;
        imshow("video capture test", image);
        waitKey(1);
    }
}

// Size.area()のテスト
void test_sizeArea(void){
    for (int y = 1; y <= 10; ++ y) {
        for (int x = 1; x <= 10; ++ x) {
            Size size(x, y);
            int sizeArea1 = size.width * size.height;
            int sizeArea2 = size.area();
            
            if (sizeArea1 != sizeArea2) {
                _print4(x, y, sizeArea1, sizeArea2);
                return;
            }
        }
    }
    
    std::cout << "size.area() is size.width * size.height" << std::endl;
}

// 要素ごとのかけ算
bool calcMultiplyEachElement(cv::Mat* om, const cv::Mat& im1, const cv::Mat& im2){
    // error processing
    if (om->size != im1.size || om->size != im2.size || im1.size != im2.size) {
        ERROR_PRINT3(*om, im1, im2);
        return false;
    }
    
    // calculation
    for (int c = 0; c < 3; ++ c) {
        om->at<double>(0, c) = im1.at<double>(0, c) * im2.at<double>(0, c);
    }
    return true;
}

// 要素ごとの割り算
bool calcDivideEachElement(cv::Mat* om, const cv::Mat& im1, const cv::Mat& im2){
    // error processing
    if (om->size() != im1.size() || om->size() != im2.size() || im1.size() != im2.size()) {
        ERROR_PRINT3(*om, im1, im2);
        return false;
    }
    
    // calculation
    for (int c = 0; c < 3; ++ c) {
        om->at<double>(0, c) = im1.at<double>(0, c) / im2.at<double>(0, c);
    }
    return true;
}

// 行列同士の比較(全く同じでなくても，大体似てたらok)
// input / m1       : 比較対象の行列
// input / m2       : 比較対象の行列
// input / thresh   : 同じかどうかの閾値
// return           : 同じ行列かどうか
bool compareMat(const cv::Mat& m1, const cv::Mat& m2, const double& thresh){
    // error processing
    if (m1.size() != m2.size()) {
        Size m1size = m1.size(), m2size = m2.size();
        ERROR_PRINT2(m1size, m2size);
        return false;
    }
    
    // calc
    double diff = 0;
    getDiffNumOfMat(&diff, m1, m2);
    //    _print(diff);
    
    //    if (sum < thresh * thresh) {
    if (diff < thresh) {
        return true;
    } else {
        return false;
    }
}
// 上のテスト
void test_compareMat(void){
    Mat m1 = (Mat_<double>(3, 3) << 1, 2, 3, 4, 5, 6, 7, 8, 9);
    Mat m2 = (Mat_<double>(3, 3) << 1, 2, 3, 4, 5, 6, 7, 8, 9);
    Mat m3 = (Mat_<double>(3, 3) << 2, 3, 4, 5, 6, 7, 8, 9, 10);
    Mat m4 = (Mat_<double>(3, 3) << 1.01, 2.01, 3.01, 4.01, 5.01, 6.01, 7.01, 8.01, 9.01);
    Mat m5 = (Mat_<double>(3, 3) << 1.0001, 2.0001, 3.0001, 4.0001, 5.0001, 6.0001, 7.0001, 8.0001, 9.0001);
    
    compareMat(m1, m2, 0.01);
    compareMat(m1, m3, 0.01);
    compareMat(m1, m4, 0.01);
    compareMat(m1, m5, 0.01);
}

// 行列の差のノルムを取得
bool getDiffNumOfMat(double* const diff, const cv::Mat& m1, const cv::Mat& m2){
    // error processing
    if (m1.size() != m2.size()) {
        Size m1size = m1.size(), m2size = m2.size();
        ERROR_PRINT2(m1size, m2size);
        return false;
    }
    
    // calc
    Mat diffMat = m1 - m2;
    *diff = cv::norm(diffMat);
    
    return true;
}

// 行列の差の絶対値の平均を求める
bool getAvgOfDiffMat(double* const diff, const cv::Mat& m1, const cv::Mat& m2){
    // error processing
    if ( !isEqualSize(m1, m2) ) return false;
    
    // init
    Mat diffMat = Mat::zeros(m1.size(), CV_64FC1);
    
    // 差の絶対値を取得
    absdiff(m1, m2, diffMat);
    
    // 上の行列の要素の平均値を取得
    reduce(diffMat, diffMat, 0, CV_REDUCE_AVG);
    *diff = diffMat.at<double>(0, 0);
    
    return true;
}
// 上のテスト
void test_getAvgOfDiffMat(void){
    Mat m1 = (Mat_<double>(3, 1) << 1, 2, 3);
    Mat m2 = (Mat_<double>(3, 1) << 1, 2, 3);
    Mat m3 = (Mat_<double>(3, 1) << 2, 3, 4);
    Mat m4 = (Mat_<double>(3, 1) << 0, 0, 0);
    Mat m5 = (Mat_<double>(3, 1) << 0.1, 0.5, -1.9);
    
    double diff = 0.0;
    getAvgOfDiffMat(&diff, m1, m2);
    _print3(diff, m1, m2);
    getAvgOfDiffMat(&diff, m1, m3);
    _print3(diff, m1, m3);
    getAvgOfDiffMat(&diff, m1, m4);
    _print3(diff, m1, m4);
    getAvgOfDiffMat(&diff, m1, m5);
    _print3(diff, m1, m5);
}

// 差の比率を取得
bool getRateOfDiffMat(double* const diff, const cv::Mat& m1, const cv::Mat& m2){
    if ( !getAvgOfDiffMat(diff, m1, m2) ) return false;
    *diff = *diff / m1.at<double>(0, 0);
    
    return true;
}

// Size::operator== の検証
void test_sizeCompare(void){
    Size s1(2, 2), s2(2, 2), s3(2, 3), s4(3, 2), s5(3, 3);
    
    if (s1 == s2) {
        std::cout << "s1 == s2" << std::endl;
    }
    if (s1 == s3) {
        std::cout << "s1 == s3" << std::endl;
    }
    if (s1 == s4) {
        std::cout << "s1 == s4" << std::endl;
    }
    if (s1 == s5) {
        std::cout << "s1 == s5" << std::endl;
    }
}

// Matの全画素にある操作を行う関数（未実装）
void doAnyMethodForAllPixelOfMat(cv::Mat* const m1){
    int rows = m1->rows, cols = m1->cols;   // 行と列の大きさ
    
    // 連続性の確認
    if (m1->isContinuous()) {
        // 連続ならばループを二重から一重に変更
        cols *= rows;
        rows = 1;
    }
    
    // 行列へアクセスし個々に変換
    for (int y = 0; y < rows; ++ y) {
        // init pointer
        double *dst = m1->ptr<double>(y);
        
        for (int x = 0; x < cols; ++ x) {
            dst[x] = 0;
        }
    }
}

// データを可視化する関数
// input / data : 可視化したいデータ
// return       : 成功したかどうか
bool showData(const cv::Mat& data){
    Mat image = Mat::zeros(Size(512, 512), CV_8UC3);
    imshow("date image", image);
    return true;
}

// 要素ごとのかけ算
bool mulElmByElm(cv::Mat* const dst, const cv::Mat& src1, const cv::Mat& src2){
    // error processing
    if ( !isEqualSize(*dst, src1, src2) ){
        std::cerr << "Mat size is diffarent" << std::endl;
        ERROR_PRINT3(*dst, src1, src2);
        return false;
    }
    
    // 連続性の確認
    int cols = dst->cols, rows = dst->rows;
    if ( isContinuous(*dst, src1, src2) ) {
        // 連続ならばループを二重から一重に変更
        cols *= rows;
        rows = 1;
    }
    
    // 行列へアクセスし個々に変換
    for (int y = 0; y < rows; ++ y) {
        // init pointer
        double *p_dst = dst->ptr<double>(y);
        const double *p_src1 = src1.ptr<double>(y);
        const double *p_src2 = src2.ptr<double>(y);
        
        for (int x = 0; x < cols; ++ x) {
            p_dst[x] = p_src1[x] * p_src2[x];
        }
    }
    
    return true;
}

// 要素ごとの割り算
bool divElmByElm(cv::Mat* const dst, const cv::Mat& src1, const cv::Mat& src2){
    // error processing
    if ( !isEqualSize(*dst, src1, src2) ){
        std::cerr << "Mat size is diffarent" << std::endl;
        ERROR_PRINT3(*dst, src1, src2);
        return false;
    }
    
    // 連続性の確認
    int cols = dst->cols, rows = dst->rows;
    if ( isContinuous(*dst, src1, src2) ) {
        // 連続ならばループを二重から一重に変更
        cols *= rows;
        rows = 1;
    }
    
    // 行列へアクセスし個々に変換
    for (int y = 0; y < rows; ++ y) {
        // init pointer
        double *p_dst = dst->ptr<double>(y);
        const double *p_src1 = src1.ptr<double>(y);
        const double *p_src2 = src2.ptr<double>(y);
        
        for (int x = 0; x < cols; ++ x) {
            p_dst[x] = p_src1[x] / p_src2[x];
        }
    }
    
    return true;
}

// 要素全体を引数で与えた色で正規化する
// output / image   : 操作する画像
bool normalizeByAnyColorChannel(cv::Mat* const image, const ColorName _cName){
    //
    int rows = image->rows, cols = image->cols, ch = image->channels();
    if ( image->isContinuous() ) {
        // 連続ならばループを二重から一重に変更
        cols *= rows;
        rows = 1;
    }
    
    // 行列へアクセスし個々に変換
    for (int y = 0; y < rows; ++ y) {
        // init pointer
        double *p_dst = image->ptr<double>(y);
        
        for (int x = 0; x < cols; ++ x) {
            // 分母の値をセット
            double divNum = p_dst[x * ch + _cName];
            
            // 分母が０の場合の処理
            if (divNum == 0) {
                // set black
                for (int c = 0; c < 3; ++ c) {
                    p_dst[x * ch + c] = 255;
                }
                p_dst[x * ch + _cName] = 1;
            } else {
                p_dst[x * ch + 0] /= divNum;
                p_dst[x * ch + 1] /= divNum;
                p_dst[x * ch + 2] /= divNum;
                //                p_dst[x * ch + 0] = 0;
                //                p_dst[x * ch + 1] = 0;
                //                p_dst[x * ch + 2] = 0;
                //                p_dst[x * ch + _cName] = 255;
            }
        }
    }
    
    return true;
}

// 画像の平均色を取得
bool calcAverageOfImage(cv::Vec3b* const _aveColor, const cv::Mat& image){
    // setting
    
    int rows = image.rows, cols = image.cols;
    if (image.isContinuous()) {
        cols *= rows;
        rows = 1;
    }
    
    // scanning all pixel
    cv::Vec3d sum(0, 0, 0);
    for (int y = 0; y < rows; ++ y) {
        const cv::Vec3b* pixVal = image.ptr<cv::Vec3b>(y);
        for (int x = 0; x < cols; ++ x) {
            sum += *(pixVal + x);
        }
    }
    sum /= rows * cols;
    _print(sum);
    *_aveColor = (cv::Vec3b)sum;
    
    return true;
}

