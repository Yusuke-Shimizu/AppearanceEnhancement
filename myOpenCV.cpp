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

// Matの差分を拡張してから入れる
void subMat(Mat *dst, const Mat* const src1, const Mat* const src2){
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
    }
    printMatPropaty(dst);std::cout << "-------------" << std::endl;
    printMatPropaty(src1);std::cout << "-------------" << std::endl;
    printMatPropaty(src2);std::cout << std::endl;
    //
    int row = dst->rows, col = dst->cols;
	//MatIterator_<unsigned char> itrSrc1 = src1->begin<unsigned char>(), itrSrc2 = src2->begin<unsigned char>();
	//MatIterator_<int> itrDest = dst->begin<int>();
    
    for (int y = 0; y < col; ++ y) {
        for (int x = 0; x < row; ++ x) {
            //dst->at<int>(x, y) = (int)(src1->at<unsigned char>(x, y) - src2->at<unsigned char>(x, y));
            int pos = x + y * row;
            dst->data[pos] = (int)src1->data[pos] - (int)src2->data[pos];
            //_print3(dst->at<int>(x, y), (int)src1->at<unsigned char>(x, y), (int)src2->at<unsigned char>(x, y));
        }
    }
}


