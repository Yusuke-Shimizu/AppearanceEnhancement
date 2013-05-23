//
//  myOpenCV.cpp
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/05/10.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//

#include "myOpenCV.h"
#include "common.h"

using namespace cv;

// セッティング
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

// Pointのセッティング
void setPoint(cv::Point* const p, const int _x, const int _y){
    p->x = _x;
    p->y = _y;
}


/**
 * 表示関数群
 */
// Matの様々な要素を表示
void printMatPropaty(const Mat* const m1){
    std::cout << "--------------------------"  << std::endl;
    // 行数
    std::cout << "rows:" << m1->rows <<std::endl;
    // 列数
    std::cout << "cols:" << m1->cols << std::endl;
    // 次元数
    std::cout << "dims:" << m1->dims << std::endl;
    // サイズ（2次元の場合）
    std::cout << "size[]:" << m1->size().width << "," << m1->size().height << std::endl;
    // ビット深度ID
    std::cout << "depth (ID):" << m1->depth() << "(=" << CV_64F << ")" << std::endl;
    // チャンネル数
    std::cout << "channels:" << m1->channels() << std::endl;
    // （複数チャンネルから成る）1要素のサイズ [バイト単位]
    std::cout << "elemSize:" << m1->elemSize() << "[byte]" << std::endl;
    // 1要素内の1チャンネル分のサイズ [バイト単位]
    std::cout << "elemSize1 (elemSize/channels):" << m1->elemSize1() << "[byte]" << std::endl;
    // 要素の総数
    std::cout << "total:" << m1->total() << std::endl;
    // ステップ数 [バイト単位]
    std::cout << "step:" << m1->step << "[byte]" << std::endl;
    // 1ステップ内のチャンネル総数
    std::cout << "step1 (step/elemSize1):" << m1->step1()  << std::endl;
    // データは連続か？
    std::cout << "isContinuous:" << (m1->isContinuous()?"true":"false") << std::endl;
    // 部分行列か？
    std::cout << "isSubmatrix:" << (m1->isSubmatrix()?"true":"false") << std::endl;
    // データは空か？
    std::cout << "empty:" << (m1->empty()?"true":"false") << std::endl;
    std::cout << "--------------------------"  << std::endl;
}

// OpenCVのバージョン表示
void printOpenCVVersion(void) {
    std::cout << "version: " << CV_VERSION << std::endl;
    std::cout << "  major: " << CV_MAJOR_VERSION << std::endl;
    std::cout << "  minor: " << CV_MINOR_VERSION << std::endl;
    std::cout << "  subminor: " << CV_SUBMINOR_VERSION << std::endl;
    std::cout << "OpenCV >= 2.0.0: " << (OPENCV_VERSION_CODE>=OPENCV_VERSION(2,0,0)?"true":"false") << std::endl;
}

// 初期化関数群
// Pointの初期化
void initPoint(cv::Point* const p, const int size){
    for (int i = 0; i < size; ++ i) {
        (p + i)->x = 0;
        (p + i)->y = 0;
    }
}

// その他の関数
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

// 符号無し8bit同士のMatの差分を符号有り16bitのMatに代入する
// dst  : 符号有り１６ビット行列
// src1 : 符号無し８ビット行列
// src2 : 符号無し８ビット行列
/*void subMat(Mat *dst, const Mat* const src1, const Mat* const src2){
    // error processing
    if (dst->rows != src1->rows || dst->rows != src1->rows || src1->rows != src2->rows) {
        ERROR_PRINT(dst->rows);
        ERROR_PRINT(src1->rows);
        ERROR_PRINT(src2->rows);
        return;
    } else if (dst->cols != src1->cols || dst->cols != src1->cols || src1->cols != src2->cols){
        ERROR_PRINT(dst->cols);
        ERROR_PRINT(src1->cols);
        ERROR_PRINT(src2->cols);
        return;
    } else if (dst->depth() != CV_16SC1) {
        int dstDepth = dst->depth();
        ERROR_PRINT(dstDepth);
        return;
    } else if (src1->depth() != CV_8UC1) {
        int src1Depth = src1->depth();
        ERROR_PRINT(src1Depth);
        return;
    } else if (src2->depth() != CV_8UC1) {
        int src2Depth = src2->depth();
        ERROR_PRINT(src2Depth);
        return;
    }
    
    //  行と列の数を取得
    int rows = dst->rows, cols = dst->cols;
    // 連続性の確認
    if (src1->isContinuous() && src2->isContinuous() && dst->isContinuous()) {
        // 連続ならばループを二重から一重に変更
        cols *= rows;
        rows = 1;
    }
    
    // 行列へアクセスし個々に変換
    for (int y = 0; y < rows; ++ y) {
        // init pointer
        const uchar *sp1 = src1->ptr<uchar>(y);
        const uchar *sp2 = src2->ptr<uchar>(y);
        short *dp = dst->ptr<short>(y);
        
        for (int x = 0; x < cols; ++ x) {
            dp[x] = (short)sp1[x] - (short)sp2[x];
        }
    }
}*/

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
    if (src16s->isContinuous() && dst8u->isContinuous()) {
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

// imwriteのテスト
void test_imwrite(void){
    
}

// 
int areaSize(const cv::Size* const _size){
    return _size->width * _size->height;
}
