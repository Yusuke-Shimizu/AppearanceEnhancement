//
//  ProCam.cpp
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/06/01.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//

#include <iostream>
#include <fstream>  // save and load method
#include <string>   // use string in load method
#include "ProCam.h"
#include "LinearizerOfProjector.h"
#include "myOpenCV.h"
#include "GeometricCalibration.h"
#include "common.h"

using namespace std;
using namespace cv;

///////////////////////////////  constructor ///////////////////////////////
// コンストラクタ
ProCam::ProCam(void)
{
    init();
}

ProCam::ProCam(const cv::Size& projectorSize)
{
    init(projectorSize);
}

ProCam::ProCam(const int _width, const int _height)
{
    init(_width, _height);
}

ProCam::ProCam(const int _size)
{
    init(_size);
}

///////////////////////////////  destructor ///////////////////////////////
// デストラクタ
ProCam::~ProCam(void){
    cout << "deleting ProCam (" << this <<")" << endl;
//    destroyWindow(WINDOW_NAME);     // 投影に使用したウィンドウを削除
    destroyAllWindows();    // ウィンドウの全削除
    DCam_stop_capture(m_dcam);
    DCam_delete(&m_dcam);
    cout << "ProCam is deleted (" << this <<")" << endl;
}

///////////////////////////////  init method ///////////////////////////////

// ProCamクラスの初期化
bool ProCam::init(const cv::Size& projectorSize){
    // カメラの初期化
    if ( !initCamera() ) return false;
    
    // プロジェクタの初期化
    if ( !initProjectorSize(projectorSize) ) return false;
    if ( !initProjectorResponseI2P() ) return false;
    if ( !initProjectorResponseP2I() ) return false;
    
    // アクセスマップの初期化
    if ( !initAccessMapCam2Prj() ) return false;
    
    return true;
}
bool ProCam::init(const int _width, const int _height){
    Size projectorSize(_width, _height);
    return init(projectorSize);
}
bool ProCam::init(const int _size) { return init(_size, _size); }
bool ProCam::init(void) { return init(100); }

// カメラの初期化
bool ProCam::initCamera(void){
//    if ( !initVideoCapture() ) return false;
//    if ( !initCameraSize() ) return false;
    if ( !initDCam() ) return false;
    if ( !initCameraSizeOfDCam1394() ) return false;
    return true;
}

// m_dcamの初期化
bool ProCam::initDCam(void){
    const CameraConfig config = config_dragonfly_vga_color;
    const int cameraNumber = 0;
    
    // DCamの設定
    m_dcam = DCam_new(config, cameraNumber);
    DCam_start_capture(m_dcam);

    return true;
}

// m_cameraSizeの初期化
// return   : 成功したかどうか
bool ProCam::initCameraSize(void){
    // videoCaptureからサイズを取得する
//    VideoCapture* l_video = getVideoCapture();
//    cv::Size videoSize(l_video->get(CV_CAP_PROP_FRAME_WIDTH), l_video->get(CV_CAP_PROP_FRAME_HEIGHT));
    Mat image;
    getCaptureImage(&image);
    const Size camSize(image.cols, image.rows);

    // setting
    if( !setCameraSize(camSize) ) return false;
    cout << "camera size is " << camSize << endl;
    
    return true;
}
bool ProCam::initCameraSizeOfDCam1394(void){
    DCam l_dcam = getDCam();
    const Size camSize(l_dcam->frame->size[0], l_dcam->frame->size[1]);
    
    // setting
    if( !setCameraSize(camSize) ) return false;
    cout << "camera size is " << camSize << endl;
    return true;
}

//
bool ProCam::initProjectorSize(const cv::Size& projectorSize){
    cout << "projector size is " << projectorSize << endl;
    return setProjectorSize(projectorSize);
}

// ビデオキャプチャーの初期化
// return   : 成功したかどうか
bool ProCam::initVideoCapture(void){
//    m_video = cv::VideoCapture(0);
    m_video.open(0);
    if( !m_video.isOpened() ){
        std::cerr << "ERROR : camera is not opened !!" << std::endl;
        exit(-1);
    }
    return true;
}

// アクセスマップの初期化
bool ProCam::initAccessMapCam2Prj(void){
    cv::Size* camSize = getCameraSize();
    m_accessMapCam2Prj = Mat::zeros(*camSize, CV_16SC1);
    return true;
}

// カメラ応答特性の初期化
// input / camResSize   : 生成する応答特性の大きさ
// return               : 成功したかどうか
bool ProCam::initCameraResponse(const int camResSize){
    // error processing
    if (camResSize <= 0) return false;
    
    m_cameraResponse = new double[camResSize];
    for (int i = 0; i < camResSize; ++ i) {
        m_cameraResponse[i] = i;
    }
    return true;
}

// プロジェクタ応答特性の初期化
// return   : 成功したかどうか
bool ProCam::initProjectorResponseI2P(void){
    // init
    const cv::Size* prjSize = getProjectorSize();
    const cv::Size prjResSize(prjSize->width * 256, prjSize->height);
    
    // set projector response
    m_projectorResponseI2P = cv::Mat_<cv::Vec3b>(prjResSize);
    initProjectorResponse(&m_projectorResponseI2P);
    return true;
}

bool ProCam::initProjectorResponseP2I(void){
    // init
    const cv::Size* prjSize = getProjectorSize();
    const cv::Size prjResSize(prjSize->width * 256, prjSize->height);
    
    // set projector response
    m_projectorResponseP2I = cv::Mat_<cv::Vec3b>(prjResSize);
    initProjectorResponse(&m_projectorResponseP2I);
    return true;
}

bool ProCam::initProjectorResponse(cv::Mat* const _prjRes){
    int rows = _prjRes->rows, cols = _prjRes->cols;
    for (int y = 0; y < rows; ++ y) {
        Vec3b* l_pPrjResP2I = _prjRes->ptr<Vec3b>(y);
        
        for (int x = 0; x < cols / 256; ++ x) {
            for (int i = 0; i < 256; ++ i) {
                l_pPrjResP2I[x * 256 + i] = Vec3b(INIT_RES_NUM, INIT_RES_NUM, INIT_RES_NUM);
            }
            l_pPrjResP2I[x * 256 + 0] = CV_VEC3B_BLACK;
            l_pPrjResP2I[x * 256 + 255] = CV_VEC3B_WHITE;
        }
    }
    return true;
}

///////////////////////////////  set method ///////////////////////////////
// m_cameraSizeの設定
bool ProCam::setCameraSize(const cv::Size& cameraSize){
    m_cameraSize = cameraSize;
    return true;
}

// m_projectorSizeの設定
bool ProCam::setProjectorSize(const cv::Size& projectorSize){
    m_projectorSize = projectorSize;
    return true;
}


bool ProCam::setAccessMapCam2Prj(const cv::Mat_<cv::Vec2i>& _accessMapCam2Prj){
    // error
    if (!isEqualSizeAndType(_accessMapCam2Prj, m_accessMapCam2Prj)) {
        cerr << "size or type is different" << endl;
        _print_mat_propaty(_accessMapCam2Prj);
        _print_mat_propaty(m_accessMapCam2Prj);
        return false;
    }
    
    // deep copy
    m_accessMapCam2Prj = _accessMapCam2Prj.clone();
    
    return true;
}

// プロジェクタ応答特性（m_projectorResponse）の設定
// input / prjResSize   : 設定したい大きさ
// return               : 成功したかどうか
bool ProCam::setProjectorResponse(const cv::Mat_<cv::Vec3b>& _response){
    // error processing
    const Mat_<Vec3b>* l_prjResI2P = getProjectorResponseI2P();
    if ( !isEqualSizeAndType(_response, *l_prjResI2P)) {
        ERROR_PRINT("_response Size or Type is different from m_projectorResponse");
        _print_mat_propaty(_response);
        _print_mat_propaty(*l_prjResI2P);
        return false;
    }
    
    // deep copy
    m_projectorResponseI2P = _response.clone();
    return true;
}

bool ProCam::setProjectorResponseP2I(const cv::Mat_<cv::Vec3b>& _response){
    // error processing
    if ( !isEqualSizeAndType(_response, m_projectorResponseP2I)) {
        ERROR_PRINT("_response Size or Type is different from m_projectorResponse");
        _print_mat_propaty(_response);
        _print_mat_propaty(m_projectorResponseP2I);
        return false;
    }
    
    // deep copy
    m_projectorResponseP2I = _response.clone();
    return true;
}

///////////////////////////////  get method ///////////////////////////////
// m_dcamの取得
DCam ProCam::getDCam(void){
    return m_dcam;
}

// m_cameraSizeの取得
//bool ProCam::getCameraSize(cv::Size* const cameraSize){
//    *cameraSize = m_cameraSize;
//    return true;
//}
cv::Size* ProCam::getCameraSize(void){
    return &m_cameraSize;
}

// カメラのピクセル数を返す
// return   : カメラの全ピクセル数
int ProCam::getPixelsOfCamera(void){
    const Size* cameraSize = getCameraSize();
    return cameraSize->area();
}

// m_projectorSizeの取得
cv::Size* ProCam::getProjectorSize(void){
    return &m_projectorSize;
}

// m_videoから撮影画像を取得
// output/image : 取得した画像を入れる行列
// return       : 成功したかどうか
bool ProCam::getCaptureImage(cv::Mat* const _image){
#ifdef LIB_DC1394_FLAG  // libdc1394を用いる場合
    DCam l_dcam = getDCam();
    captureCVImage(_image, l_dcam);
#else
    VideoCapture* l_video = getVideoCapture();
#ifdef BAYER_FLAG       // カメラ入力がbayer配列の場合
    Mat l_bayerImage;
    *l_video >> l_bayerImage;
    cv::cvtColor(l_bayerImage, *_image, CV_BayerBG2RGB);
#else                   // OpenCVで処理出来る場合
    *l_video >> *_image;
#endif
#endif
    return true;
}

// m_videoの取得
// output / _video
cv::VideoCapture* ProCam::getVideoCapture(void){
    return &m_video;
}

// m_accessMapCam2Proの取得
const cv::Mat_<cv::Vec2i>* ProCam::getAccessMapCam2Prj(void){
    return &m_accessMapCam2Prj;
}

const cv::Mat_<cv::Vec3b>* ProCam::getProjectorResponseI2P(void){
    return &m_projectorResponseI2P;
}
const cv::Mat_<cv::Vec3b>* ProCam::getProjectorResponseP2I(void){
    return &m_projectorResponseP2I;
}

// 引数で与えられた応答特性マップのある輝度の画像を取得
void ProCam::getImageProjectorResponseP2I(cv::Mat* const _responseImage, const cv::Mat& _responseMap, const int _index){
    // error processing
    if (_responseImage->rows != _responseMap.rows || _responseImage->cols != _responseMap.cols / 256) {
        cerr << "different size" << endl;
        _print_mat_propaty(*_responseImage);
        _print_mat_propaty(_responseMap);
        exit(-1);
    }
    const int rows = _responseImage->rows, cols = _responseImage->cols;
    for (int y = 0; y < rows; ++ y) {
        // init pointer
        const Vec3b* l_pPrjResP2I = _responseMap.ptr<Vec3b>(y);
        Vec3b* l_pResImg = _responseImage->ptr<Vec3b>(y);
        
        for (int x = 0; x < cols; ++ x) {
            l_pResImg[x] = l_pPrjResP2I[x * 256 + _index];
        }
    }
}

// 応答特性マップのある輝度の時の画像を取得する
void ProCam::getImageProjectorResponseP2I(cv::Mat* const _responseImage, const int _index){
    const Mat_<Vec3b>* l_prjResP2I = getProjectorResponseP2I();
    getImageProjectorResponseP2I(_responseImage, *l_prjResP2I, _index);
}

///////////////////////////////  save method ///////////////////////////////
bool ProCam::saveAccessMapCam2Prj(void){
    // init
    cout << "saving look up table..." << endl;
    const Mat_<Vec2i>* l_accessMapC2P = getAccessMapCam2Prj();
    
    // バイナリ出力モード（初期化を行う）
    ofstream ofs;
    ofs.open(LOOK_UP_TABLE_FILE_NAME, ios_base::out | ios_base::trunc | ios_base::binary);
    if (!ofs) {
        ERROR_PRINT2(LOOK_UP_TABLE_FILE_NAME, "is Not Found");
        exit(-1);
    }
    
    // write look up table size
    int rows = l_accessMapC2P->rows, cols = l_accessMapC2P->cols;
    ofs.write((char*)&rows, sizeof(int));
    ofs.write((char*)&cols, sizeof(int));
    
    // write table
    if (l_accessMapC2P->isContinuous()) {
        cols *= rows;
        rows = 1;
    }
    Point tmp(0, 0);
    for (int y = 0; y < rows; ++ y) {
        const Vec2i* l_pAccessMapC2P = l_accessMapC2P->ptr<Vec2i>(y);
        
        for (int x = 0; x < cols; ++ x) {
            tmp = Point(l_pAccessMapC2P[x]);
            ofs.write((char*)&tmp.x, sizeof(int));
            ofs.write((char*)&tmp.y, sizeof(int));
        }
    }
    
    cout << "finish saving!" << endl;
    return true;
}

// 
bool ProCam::saveProjectorResponseForByte(const char* fileName){
    // init
    const Mat_<Vec3b>* const l_proRes = getProjectorResponseI2P();
    ofstream ofs;
    ofs.open(fileName, ios_base::out | ios_base::trunc | ios_base::binary);
    if (!ofs) {
        ERROR_PRINT2(fileName, "is Not Found");
        exit(-1);
    }
    
    // write
    int rows = l_proRes->rows, cols = l_proRes->cols;
    ofs.write((char*)&rows, sizeof(int));
    ofs.write((char*)&cols, sizeof(int));
    for (int y = 0; y < rows; ++ y) {
        const Vec3b* p_proRes = l_proRes->ptr<Vec3b>(y);
        for (int x = 0; x < cols; ++ x) {
            for (int ch = 0; ch < 3; ++ ch) {
                ofs.write((char*)&p_proRes[x][ch], sizeof(uchar));
            }
        }
    }
    
    return true;
}

bool ProCam::saveProjectorResponseP2IForByte(const char* fileName){
    // init
    const Mat_<Vec3b>* const l_proRes = getProjectorResponseP2I();
    ofstream ofs;
    ofs.open(fileName, ios_base::out | ios_base::trunc | ios_base::binary);
    if (!ofs) {
        ERROR_PRINT2(fileName, "is Not Found");
        exit(-1);
    }
    
    // write
    int rows = l_proRes->rows, cols = l_proRes->cols;
    ofs.write((char*)&rows, sizeof(int));
    ofs.write((char*)&cols, sizeof(int));
    for (int y = 0; y < rows; ++ y) {
        const Vec3b* p_proRes = l_proRes->ptr<Vec3b>(y);
        for (int x = 0; x < cols; ++ x) {
            for (int ch = 0; ch < 3; ++ ch) {
                ofs.write((char*)&p_proRes[x][ch], sizeof(uchar));
            }
        }
    }
    
    return true;
}

// printProjectorResponseの内容を保存
bool ProCam::savePrintProjectorResponse(const char* fileName, const cv::Point& _pt, const cv::Mat& _prjRes){
    Mat l_responseImage(_prjRes.rows, _prjRes.cols / 256, CV_8UC3, Scalar(0, 0, 0));
    ofstream ofs(fileName);
    if (!ofs) {
        cout << fileName << "is not found" << endl;
        exit(-1);
    }
    
    for (int i = 0; i < 256; ++ i) {
        getImageProjectorResponseP2I(&l_responseImage, _prjRes, i);
        
        Vec3i tmp = (Vec3i)l_responseImage.at<Vec3b>(_pt);
        _print_gnuplot4(ofs, i, tmp[2], tmp[1], tmp[0]);
    }
    
    return true;
}
bool ProCam::savePrintProjectorResponseI2P(const char* fileName, const cv::Point& _pt){
    const Mat_<Vec3b>* l_responseMap = getProjectorResponseI2P();

    return savePrintProjectorResponse(fileName, _pt, *l_responseMap);
}
bool ProCam::savePrintProjectorResponseP2I(const char* fileName, const cv::Point& _pt){
    const Mat_<Vec3b>* l_responseMap = getProjectorResponseP2I();
    
    return savePrintProjectorResponse(fileName, _pt, *l_responseMap);
}

///////////////////////////////  load method ///////////////////////////////
bool ProCam::loadAccessMapCam2Prj(void){
    cout << "loading look up table..." << endl;
    // init
    ifstream ifs(LOOK_UP_TABLE_FILE_NAME);
    if (!ifs) {
        cerr << LOOK_UP_TABLE_FILE_NAME << "is not found" << endl;
        exit(-1);
    }
    
    // load size
    Size mapSize(0, 0);
    ifs.read((char*)&mapSize.height, sizeof(int));
    ifs.read((char*)&mapSize.width, sizeof(int));
    
    // init access map
    Mat_<Vec2i> l_accessMapC2P = Mat::zeros(mapSize, CV_16SC2);
    
    // load
    int rows = l_accessMapC2P.rows, cols = l_accessMapC2P.cols;
    Point tmp(0, 0);
    for (int y = 0; y < rows; ++ y) {
        Vec2i* l_pAccessMapC2P = l_accessMapC2P.ptr<Vec2i>(y);
        
        for (int x = 0; x < cols; ++ x) {
            ifs.read((char*)&tmp.x, sizeof(int));
            ifs.read((char*)&tmp.y, sizeof(int));
            l_pAccessMapC2P[x] = Vec2i(tmp);
        }
    }
    
    // setting look up table
    setAccessMapCam2Prj(l_accessMapC2P);
    
    cout << "finish loading!" << endl;
    return true;
}

// load response function of projector
bool ProCam::loadProjectorResponseForByte(const char* fileName){
    // init
    ifstream ifs(fileName);
    if (!ifs) {
        ERROR_PRINT2(fileName, "is Not Found");
        exit(-1);
    }
    
    // get size
    int rows = 0, cols = 0;
    ifs.read((char*)&rows, sizeof(int));
    ifs.read((char*)&cols, sizeof(int));
    Mat_<Vec3b> l_proRes(rows, cols, CV_8UC3);
    
    // get number
    for (int y = 0; y < rows; ++ y) {
        Vec3b* p_proRes = l_proRes.ptr<Vec3b>(y);
        
        for (int x = 0; x < cols; ++ x) {
            for (int ch = 0; ch < 3; ++ ch) {
                ifs.read((char*)&p_proRes[x][ch], sizeof(uchar));
            }
        }
    }
    
    // setting
    setProjectorResponse(l_proRes);
    return true;
}

bool ProCam::loadProjectorResponseP2IForByte(const char* fileName){
    // init
    ifstream ifs(fileName);
    if (!ifs) {
        ERROR_PRINT2(fileName, "is Not Found");
        exit(-1);
    }
    
    // get size
    int rows = 0, cols = 0;
    ifs.read((char*)&rows, sizeof(int));
    ifs.read((char*)&cols, sizeof(int));
    Mat_<Vec3b> l_proRes(rows, cols, CV_8UC3);
    
    // get number
    for (int y = 0; y < rows; ++ y) {
        Vec3b* p_proRes = l_proRes.ptr<Vec3b>(y);
        
        for (int x = 0; x < cols; ++ x) {
            for (int ch = 0; ch < 3; ++ ch) {
                ifs.read((char*)&p_proRes[x][ch], sizeof(uchar));
            }
        }
    }
    
    // setting
    setProjectorResponseP2I(l_proRes);
    return true;
}

///////////////////////////////  calibration method ///////////////////////////////
// 全てのキャリブレーションを行う
bool ProCam::allCalibration(void){
    // geometri calibration
    if ( !geometricCalibration() ) {
        cerr << "geometric calibration error" << endl;
        return false;
    }

    // linearized projector
    if ( !linearizeOfProjector() ) {
        cerr << "linearized error of projector" << endl;
        exit(-1);
    }

    // color caliration
    if ( !colorCalibration() ) {
        cerr << "color caliration error" << endl;
        exit(-1);
    }
    
    return true;
}

// 幾何キャリブレーションを行う
bool ProCam::geometricCalibration(void){
    // init
    Size* camSize = getCameraSize();
    Mat_<Vec2i> l_accessMapCam2Prj(*camSize);
    
#ifdef GEO_CAL_CALC_FLAG
    // calculation
    GeometricCalibration gc(this);
    if (!gc.doCalibration(&l_accessMapCam2Prj)) return false;
    setAccessMapCam2Prj(l_accessMapCam2Prj);
    saveAccessMapCam2Prj();
#else
    // load
    loadAccessMapCam2Prj();
#endif
    // show geometric map
    showAccessMapCam2Prj();
    
    return true;
}

// プロジェクタの線形化を行う
// return   : 成功したかどうか
bool ProCam::linearizeOfProjector(void){
    // init
    const Mat_<Vec3b>* l_prjRes = getProjectorResponseI2P();
    int rows = l_prjRes->rows, cols = l_prjRes->cols;
    Mat_<Vec3b> l_prjResponseI2P(rows, cols);
    Mat_<Vec3b> l_prjResponseP2I(rows, cols);
    initProjectorResponse(&l_prjResponseI2P);
    initProjectorResponse(&l_prjResponseP2I);
    
    // get projector response
    LinearizerOfProjector linearPrj(this);
#ifdef PRJ_LINEAR_CALC_FLAG
    if ( !linearPrj.doLinearlize(&l_prjResponseI2P, &l_prjResponseP2I) ) return false;    // 引数消してもいいかも
#else
    linearPrj.loadColorMixingMatrixOfByte(CMM_MAP_FILE_NAME_BYTE);
    loadProjectorResponseP2IForByte(PROJECTOR_RESPONSE_P2I_FILE_NAME_BYTE);
    loadProjectorResponseForByte(PROJECTOR_RESPONSE_I2P_FILE_NAME_BYTE);
    linearPrj.loadAllCImages();
#endif
    
    // show
    showProjectorResponseP2I();
    showProjectorResponseI2P();
    
    // print
    Point pt(cols*0.6/256, rows*0.6);
    printProjectorResponseP2I(pt);
    printProjectorResponseI2P(pt);
    savePrintProjectorResponseP2I(PROJECTOR_RESPONSE_AT_SOME_POINT_P2I_FILE_NAME, pt);
    savePrintProjectorResponseI2P(PROJECTOR_RESPONSE_AT_SOME_POINT_I2P_FILE_NAME, pt);
    linearPrj.saveAllC(PROJECTOR_ALL_C_IMAGES_FILE_NAME, pt);

    // test
    cout << "do radiometric compensation" << endl;
    int prjLum = 0;
    linearPrj.doRadiometricCompensation(prjLum);
    while (true) {
        linearPrj.doRadiometricCompensation(prjLum);
        prjLum += 1;    //1 or 10
        if (prjLum >= 256) {
            prjLum = 0;
            break;
        }
        if (waitKey(5) == CV_BUTTON_ESC) break;
    }
    cout << "did radiometric compensation" << endl;
    return true;
}

// 光学キャリブレーションを行う
bool ProCam::colorCalibration(void){
    cout << "color calibration start!" << endl;
    
    
    
    cout << "color calibration finish!" << endl;
    return true;
}

///////////////////////////////  convert method ///////////////////////////////

// 幾何変換テーブルを用いて，カメラ座標系からプロジェクタ座標系に変換する
// output / _psImg  : プロジェクタ座標系に投影した画像
// input / _csImg   : カメラ座標系の画像
bool ProCam::convertProjectorDomainToCameraOne(cv::Mat* const _psImg, const cv::Mat&  _csImg){
    // error processing
    const Mat_<Vec2i>* l_accessMapC2P = getAccessMapCam2Prj();              // カメラ空間からプロジェクタ空間への変換テーブル
    if ( !isEqualSize(*l_accessMapC2P, _csImg) ) {
        cerr << "size is different" << endl;
        _print_mat_propaty(_csImg);
        _print_mat_propaty(*l_accessMapC2P);
        exit(-1);
    }
    const Size* l_prjSize = getProjectorSize();
    const Size l_psImgSize(_psImg->cols, _psImg->rows);
    if (*l_prjSize != l_psImgSize) {
        _print_mat_propaty(*_psImg);
        ERROR_PRINT2(*l_prjSize, l_psImgSize);
        exit(-1);
    }
    
    // camera coordinate system -> projector coordinate system
    Mat l_psImg(_psImg->rows, _psImg->cols, CV_8UC3, Scalar(0, 0, 0));  // プロジェクタ空間上の画像（ローカル）
    int rows = _csImg.rows, cols = _csImg.cols;
    if (isContinuous(*l_accessMapC2P, _csImg) ) {
        cols *= rows;
        rows = 1;
    }
    for (int y = 0; y < rows; ++ y) {
        const Vec2i* l_pAccessMapC2P = l_accessMapC2P->ptr<Vec2i>(y);
        const Vec3b* l_pCsImg = _csImg.ptr<Vec3b>(y);
        
        for (int x = 0; x < cols; ++ x) {
            Point prjPoint = Point(l_pAccessMapC2P[x]); // projector coordinate system
            l_psImg.at<Vec3b>(prjPoint) = l_pCsImg[x];  // set projector image
        }
    }
    
    // deep copy
    *_psImg = l_psImg.clone();
    
    return true;
}


// プロジェクタの強度が線形化されていない画像から線形化された画像へ変換
// output / _linearImg      : 線形化後の画像
// input / _nonLinearImg    : 線形化前の画像
bool ProCam::convertNonLinearImageToLinearOne(cv::Mat* const _linearImg, const cv::Mat&  _nonLinearImg){
    // error processing
    const Mat_<Vec3b>* l_prjRes = getProjectorResponseI2P();               // プロジェクタ強度の線形化ルックアップテーブル
    const Mat l_compressedPrjRes(l_prjRes->rows, l_prjRes->cols / 256, CV_8UC3);
    if (!isEqualSize(*_linearImg, _nonLinearImg, l_compressedPrjRes)) {
        cerr << "different size" << endl;
        _print_mat_propaty(*_linearImg);
        _print_mat_propaty(_nonLinearImg);
        _print_mat_propaty(l_compressedPrjRes);
        exit(-1);
    }
    
    // init lineared image
    int rows = _nonLinearImg.rows, cols = _nonLinearImg.cols, imgCh = _nonLinearImg.channels(); // 投影サイズ
    Mat l_linearImage(rows, cols, CV_8UC3, Scalar(0, 0, 0));    // プロジェクタ強度の線形化を行った後の投影像
    
    // scan all pixel and channel
    for (int y = 0; y < rows; ++ y) {
        // init pointer
        const Vec3b* l_pPrjRes = l_prjRes->ptr<Vec3b>(y);
        const Vec3b* l_pNonLinearImg = _nonLinearImg.ptr<Vec3b>(y);
        Vec3b* l_pLinearImg = _linearImg->ptr<Vec3b>(y);
        
        for (int x = 0; x < cols; ++ x) {
            for (int ch = 0; ch < imgCh; ++ ch) {
                const int prjResIndex = x * 256 + l_pNonLinearImg[x][ch];   // 応答特性マップの座標
                l_pLinearImg[x][ch] = l_pPrjRes[prjResIndex][ch];
            }
        }
    }
    
    // deep copy
    *_linearImg = l_linearImage.clone();
    
    return true;
}

// PからIに変換する
bool ProCam::convertPtoI(cv::Mat* const _I, const cv::Mat&  _P){
    // error processing
    if (!isEqualSizeAndType(*_I, _P)) {
        cerr << "different size or type" << endl;
        _print_mat_propaty(*_I);
        _print_mat_propaty(_P);
        exit(-1);
    }
    
    // scanning all pixel
    const Mat* l_transP2I = getProjectorResponseP2I();
    const int rows = _I->rows, cols = _I->cols, ch = _I->channels();
    for (int y = 0; y < rows; ++ y) {
        Vec3b* l_pI = _I->ptr<Vec3b>(y);
        const Vec3b* l_pP = _P.ptr<Vec3b>(y);
        const Vec3b* l_pTransP2I = l_transP2I->ptr<Vec3b>(y);
        
        for (int x = 0; x < cols; ++ x) {
            for (int c = 0; c < ch; ++ c) {
                // convert
                const int index = x * 256 + (int)l_pP[x][c];
                l_pI[x][c] = l_pTransP2I[index][c];
            }
        }
    }
    
    return true;
}

///////////////////////////////  show method ///////////////////////////////

// アクセスマップの表示
bool ProCam::showAccessMapCam2Prj(void){
    const Size* prjSize = getProjectorSize();
    const Size* camSize = getCameraSize();
    bool flag = true;
    Point pt(camSize->height / 2, camSize->width / 2);
    Mat l_capImage(*camSize, CV_8UC3);

    while (flag) {
        // init point
        Point l_startPt(pt.x - 1, pt.y - 1), l_endPt(pt.x + 1, pt.y + 1);
        
        // create projection image
        Mat whiteImg(*camSize, CV_8UC3, Scalar(255, 255, 255)), prjImg(*prjSize, CV_8UC3, Scalar(0, 0, 0));
        cv::rectangle(whiteImg, l_startPt, l_endPt, CV_SCALAR_BLUE, -1, CV_AA);
        
        // capture
        captureFromLightOnProjectorDomain(&l_capImage, whiteImg);

        // add color for capture image
        cv::rectangle(l_capImage, cv::Point(pt.x - 1,pt.y - 1), cv::Point(pt.x + 1, pt.y + 1), CV_SCALAR_RED, -1, CV_AA);

        MY_IMSHOW(l_capImage);

        int pushKey = waitKey(30);
        switch (pushKey) {
            case (CV_BUTTON_ESC):
                flag = false;
                break;
            case (CV_BUTTON_UP):
                pt.y -= 10;
                break;
            case (CV_BUTTON_LEFT):
                pt.x -= 10;
                break;
            case (CV_BUTTON_DOWN):
                pt.y += 10;
                break;
            case (CV_BUTTON_RIGHT):
                pt.x += 10;
                break;
                
            default:
                break;
        }
    }

    return true;
}

// response mapの表示
bool ProCam::showProjectorResponseI2P(void){
    const Mat_<Vec3b>* l_responseMap = getProjectorResponseI2P();
    
    return showProjectorResponse(*l_responseMap);
}

bool ProCam::showProjectorResponseP2I(void){
    const Mat_<Vec3b>* l_responseMap = getProjectorResponseP2I();
        
    return showProjectorResponse(*l_responseMap);
}

bool ProCam::showProjectorResponse(const cv::Mat& _prjRes){
    Mat l_responseImage(_prjRes.rows, _prjRes.cols / 256, CV_8UC3, Scalar(0, 0, 0));
    
    for (int i = 0;;) {
        _print(i);
        getImageProjectorResponseP2I(&l_responseImage, _prjRes, i);
        Mat l_flatImage(l_responseImage.rows, l_responseImage.cols, CV_8UC3, Scalar(i, i, i));
        
        MY_IMSHOW(l_flatImage);
        MY_IMSHOW(l_responseImage);
        if (i > 255) i = 0;
        if (i < 0) i = 255;
        
        // handle image
        int pushKey = waitKey(0);
        if (pushKey == CV_BUTTON_LEFT || pushKey == CV_BUTTON_DOWN){
            ++ i;
        }
        if (pushKey == CV_BUTTON_RIGHT || pushKey == CV_BUTTON_UP){
            -- i;
        }
        if (pushKey == CV_BUTTON_ESC) {
            cout<<"finish show image"<<endl;break;
        }
    }
    destroyAllWindows();
    
    return true;
}

///////////////////////////////  print method ///////////////////////////////

// projector responseの出力
bool ProCam::printProjectorResponseI2P(const cv::Point& _pt){
    const Mat_<Vec3b>* l_responseMap = getProjectorResponseI2P();
    
    return printProjectorResponse(_pt, *l_responseMap);
}
//
bool ProCam::printProjectorResponseP2I(const cv::Point& _pt){
    const Mat_<Vec3b>* l_responseMap = getProjectorResponseP2I();
    
    return printProjectorResponse(_pt, *l_responseMap);
}
bool ProCam::printProjectorResponse(const cv::Point& _pt, const cv::Mat& _prjRes){
    Mat l_responseImage(_prjRes.rows, _prjRes.cols / 256, CV_8UC3, Scalar(0, 0, 0));
    
    for (int i = 0; i < 256; ++ i) {
        getImageProjectorResponseP2I(&l_responseImage, _prjRes, i);
        
        Vec3i tmp = (Vec3i)l_responseImage.at<Vec3b>(_pt);
        _print_gnuplot4(std::cout, i, tmp[2], tmp[1], tmp[0]);
    }
    
    return true;
}

// videoCaptureのプロパティ
void ProCam::printVideoPropaty(void){
    VideoCapture* l_video = getVideoCapture();
    std::cout << "CV_CAP_PROP_POS_MSEC = " << l_video->get(CV_CAP_PROP_POS_MSEC) << std::endl;
    std::cout << "CV_CAP_PROP_POS_FRAMES = " << l_video->get(CV_CAP_PROP_POS_FRAMES) << std::endl;
    std::cout << "CV_CAP_PROP_POS_AVI_RATIO = " << l_video->get(CV_CAP_PROP_POS_AVI_RATIO) << std::endl;
    std::cout << "CV_CAP_PROP_FRAME_WIDTH = " << l_video->get(CV_CAP_PROP_FRAME_WIDTH) << std::endl;
    std::cout << "CV_CAP_PROP_FRAME_HEIGHT = " << l_video->get(CV_CAP_PROP_FRAME_HEIGHT) << std::endl;
    std::cout << "CV_CAP_PROP_FPS = " << l_video->get(CV_CAP_PROP_FPS) << std::endl;
    std::cout << "CV_CAP_PROP_FOURCC = " << l_video->get(CV_CAP_PROP_FOURCC) << std::endl;
    std::cout << "CV_CAP_PROP_FRAME_COUNT = " << l_video->get(CV_CAP_PROP_FRAME_COUNT) << std::endl;
    std::cout << "CV_CAP_PROP_FORMAT = " << l_video->get(CV_CAP_PROP_FORMAT) << std::endl;
    std::cout << "CV_CAP_PROP_MODE = " << l_video->get(CV_CAP_PROP_MODE) << std::endl;
    std::cout << "CV_CAP_PROP_BRIGHTNESS = " << l_video->get(CV_CAP_PROP_BRIGHTNESS) << std::endl;
    std::cout << "CV_CAP_PROP_CONTRAST = " << l_video->get(CV_CAP_PROP_CONTRAST) << std::endl;
    std::cout << "CV_CAP_PROP_SATURATION = " << l_video->get(CV_CAP_PROP_SATURATION) << std::endl;
    std::cout << "CV_CAP_PROP_HUE = " << l_video->get(CV_CAP_PROP_HUE) << std::endl;
    std::cout << "CV_CAP_PROP_GAIN = " << l_video->get(CV_CAP_PROP_GAIN) << std::endl;
    std::cout << "CV_CAP_PROP_EXPOSURE = " << l_video->get(CV_CAP_PROP_EXPOSURE) << std::endl;
    std::cout << "CV_CAP_PROP_CONVERT_RGB = " << l_video->get(CV_CAP_PROP_CONVERT_RGB) << std::endl;
    std::cout << "CV_CAP_PROP_RECTIFICATION = " << l_video->get(CV_CAP_PROP_RECTIFICATION) << std::endl;
}

///////////////////////////////  capture from light method ///////////////////////////////

// output / captureImage    : 撮影した画像を代入する場所
// input / projectionImage  : 投影する画像
// return                   : 成功したかどうか
bool ProCam::captureFromLight(cv::Mat* const captureImage, const cv::Mat& projectionImage, const int _waitTimeNum){
    // 投影
    imshow(WINDOW_NAME, projectionImage);
    cvMoveWindow(WINDOW_NAME, POSITION_OF_PROJECTION_IMAGE.x, POSITION_OF_PROJECTION_IMAGE.y);
    cv::waitKey(_waitTimeNum);
    
    // N回撮影する
    cv::Mat image;
    for (int i = 0; i < CAPTURE_NUM; ++ i) {
        getCaptureImage(&image);
    }
    
    *captureImage = image.clone();
    cv::waitKey(_waitTimeNum);
    
    return true;
}

bool ProCam::captureFromFlatLight(cv::Mat* const captureImage, const cv::Vec3b& projectionColor, const int _waitTimeNum){
    const Size* l_prjSize = getProjectorSize();
    const Mat l_projectionImage(*l_prjSize, CV_8UC3, Scalar(projectionColor));
    return captureFromLight(captureImage, l_projectionImage, _waitTimeNum);
}
bool ProCam::captureFromFlatGrayLight(cv::Mat* const captureImage, const uchar& projectionNum, const int _waitTimeNum){
    const Vec3b l_projectionColor(projectionNum, projectionNum, projectionNum);
    return captureFromFlatLight(captureImage, l_projectionColor, _waitTimeNum);
}

bool ProCam::captureFromLightOnProjectorDomain(cv::Mat* const captureImage, const cv::Mat& projectionImage, const int _waitTimeNum){
    // error processing
    const Size* l_camSize = getCameraSize();
    if (projectionImage.rows != l_camSize->height || projectionImage.cols != l_camSize->width) {
        cout << "size is different" << endl;
        _print_mat_propaty(projectionImage);
        _print(*l_camSize);
        exit(-1);
    }
    
    const Size* l_prjSize = getProjectorSize();
    Mat l_projectionImageOnProjectorSpace(*l_prjSize, CV_8UC3, Scalar(0, 0, 0));
    convertProjectorDomainToCameraOne(&l_projectionImageOnProjectorSpace, projectionImage);
    return captureFromLight(captureImage, l_projectionImageOnProjectorSpace, _waitTimeNum);
}
bool ProCam::captureFromFlatLightOnProjectorDomain(cv::Mat* const _captureImage, const cv::Vec3b& _projectionColor, const int _waitTimeNum){
    const Size* l_camSize = getCameraSize();    // カメラサイズの取得
    const Mat l_projectionImage(*l_camSize, CV_8UC3, Scalar(_projectionColor)); // 投影画像
    return captureFromLightOnProjectorDomain(_captureImage, l_projectionImage, _waitTimeNum);
}
bool ProCam::captureFromFlatGrayLightOnProjectorDomain(cv::Mat* const _captureImage, const uchar _projectionNumber, const int _waitTimeNum){
    const Vec3b l_projectionColor(_projectionNumber, _projectionNumber, _projectionNumber);
    return captureFromFlatLightOnProjectorDomain(_captureImage, l_projectionColor, _waitTimeNum);
}

// 線形化したプロジェクタを用いて投影・撮影を行う
bool ProCam::captureFromLinearLight(cv::Mat* const captureImage, const cv::Mat& projectionImage, const int _waitTimeNum){
    // init lineared projection image
    Mat l_linearProjectionImage(projectionImage.rows, projectionImage.cols, CV_8UC3, Scalar(0, 0, 0));    // プロジェクタ強度の線形化を行った後の投影像
    
    // non linear image -> linear one
    convertNonLinearImageToLinearOne(&l_linearProjectionImage, projectionImage);
    
    return captureFromLight(captureImage, l_linearProjectionImage, _waitTimeNum);
}

///////////////////////////////  other method ///////////////////////////////

// projector responseの補間を行う
bool ProCam::interpolationProjectorResponseP2I(cv::Mat* const _prjRes){
    cout << "interpolation now ..." << endl;
    // initialize
    int rows = _prjRes->rows, cols = _prjRes->cols / 256, channels = _prjRes->channels();
    if (_prjRes->isContinuous()) {
        cols *= rows;
        rows = 1;
    }
    
    // scan all pixel
    for (int y = 0; y < rows; ++ y) {
        Vec3b* l_pPrjRes = _prjRes->ptr<Vec3b>(y);
        for (int x = 0; x < cols; ++ x) {
            
            // scan all color
            for (int c = 0; c < channels; ++ c) {
                // scan all luminance
                for (int p = 1; p < 255; ++ p) {    // 最初と最後は初期化の時に決定済みである為，除外
                    if (l_pPrjRes[x * 256 + p][c] == INIT_RES_NUM) {   // 抜けている箇所の特定
                        uchar p0 = p - 1, i0 = l_pPrjRes[x * 256 + p0][c];  // 下端の値

                        // 上端の検索
                        uchar p1 = p + 1, i1 = 0;
                        for (; ; ++ p1) {
                            if (l_pPrjRes[x * 256 + p1][c] != INIT_RES_NUM || p1 == 255) {
                                i1 = l_pPrjRes[x * 256 + p1][c];
                                break;
                            }
                        }
                        if (p == 1 && p1 == 255) {
                            l_pPrjRes[x * 256 + p1][c] = INIT_RES_NUM;
                            break;
                        }
                        
                        // 抜けていた箇所全てを修復
                        for (int p_current = p; p_current < p1; ++ p_current) {
                            // set alpha
                            double alpha = (double)(p_current - p0)/(double)(p1 - p0);
                            
                            // 抜けている箇所の修復
                            l_pPrjRes[x * 256 + p_current][c] = i0 + (uchar)(alpha * (double)(i1 - i0));
                        }
                    }
                }
            }
        }
    }
    
    cout << "finished interpolation" << endl;

    return true;
}
// test of front method
bool ProCam::test_interpolationProjectorResponseP2I(void){
    Mat m1(2, 256, CV_8UC3, Scalar(INIT_RES_NUM, INIT_RES_NUM, INIT_RES_NUM));
//    m1.at<Vec3b>(0, 255) = Vec3b(127, 127, 127);
//    m1.at<Vec3b>(1, 255) = Vec3b(0, 0, 0);
    for (int i = 0; i < 256; i += 5) {
        m1.at<Vec3b>(0, i) = Vec3b(i, i, i);
        m1.at<Vec3b>(1, i) = Vec3b(0, 0, 0);
    }
    m1.at<Vec3b>(1, 255) = Vec3b(255, 255, 255);
    _print(m1);
    interpolationProjectorResponseP2I(&m1);
    _print(m1);
    
    return true;
}

// カメラが線形であるかを確かめる
bool ProCam::checkCameraLinearity(void){
    Mat image;
    getCaptureImage(&image);
    
    Vec3b* p_image = image.ptr<Vec3b>(image.rows * 0.5);
    for (int x = 0; x < image.cols; ++ x) {
        cout << p_image[x] << endl;
    }
    return true;
}
