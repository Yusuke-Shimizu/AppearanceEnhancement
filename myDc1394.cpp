//
//  myDc1394.cpp
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/08/03.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//

#include "myDc1394.h"
#include "myOpenCV.h"
#include <iostream>

using namespace std;
using namespace cv;

///////////////////////////////  convert method ///////////////////////////////

// dc1394 -> OpenCV
void convertDC1394imageToCVimage(cv::Mat* const _imageCV, const uchar* const _imageDC){
    int rows = _imageCV->rows, cols = _imageCV->cols, channels = _imageCV->channels();
    if(_imageCV->isContinuous()){
        cols *= rows;
        rows = 1;
    }
    
    for (int y = 0; y < rows; ++ y) {
        Vec3b* l_pImage = _imageCV->ptr<Vec3b>(y);
        
        for (int x = 0; x < cols; ++ x) {
            for (int c = 0; c < channels; ++ c) {
                const int pt_dcImage = c + channels * (x + cols * y);
                l_pImage[x][channels - c - 1] = _imageDC[pt_dcImage];
            }
        }
    }
}

// // OpenCV -> dc1394
void convertCVimageToDC1394image(uchar* const _imageDC, const cv::Mat& _imageCV){
    int rows = _imageCV.rows, cols = _imageCV.cols, channels = _imageCV.channels();
    if(_imageCV.isContinuous()){
        cols *= rows;
        rows = 1;
    }
    
    for (int y = 0; y < rows; ++ y) {
        const Vec3b* l_pImage = _imageCV.ptr<Vec3b>(y);
        
        for (int x = 0; x < cols; ++ x) {
            for (int c = 0; c < channels; ++ c) {
                const int pt_dcImage = c + channels * (x + cols * y);
                _imageDC[pt_dcImage] = l_pImage[x][channels - c - 1];
            }
        }
    }
}


///////////////////////////////  print method ///////////////////////////////
// カメラの特徴を出力
void printAllCameraFeatures(dc1394camera_t& _camera){
    dc1394featureset_t features;  // カメラの特徴
    dc1394error_t err = dc1394_feature_get_all(&_camera, &features);  // 特徴を取得
    if (err == DC1394_SUCCESS) {
        dc1394_feature_print_all(&features, stdout);
    } else {
        dc1394_log_warning("Could not get feature set");
    }
}


///////////////////////////////  other method ///////////////////////////////
/*-----------------------------------------------------------------------
 *  Releases the cameras and exits
 *-----------------------------------------------------------------------*/
static void cleanup_and_exit(dc1394camera_t *camera)
{
    dc1394_video_set_transmission(camera, DC1394_OFF);
    dc1394_capture_stop(camera);
    dc1394_camera_free(camera);
    exit(1);
}

static dc1394error_t setup_capture(dc1394camera_t * camera, const CameraConfig config) {
    dc1394error_t err;
    
    if(config.need_reset) {
        err = dc1394_camera_reset(camera);
        DC1394_ERR_RTN(err, "Cannot initialize camera to factory default settings");
    }
    
    // 1394b モードへ移行する
    if(config.speed >= DC1394_ISO_SPEED_800) {
        err=dc1394_video_set_operation_mode(camera, DC1394_OPERATION_MODE_1394B);
        DC1394_ERR_CLN_RTN(err,cleanup_and_exit(camera),"Could not set to 1394B mode");
    }
    err=dc1394_video_set_iso_speed(camera, config.speed);
    DC1394_ERR_CLN_RTN(err,cleanup_and_exit(camera),"Could not set iso speed");
    
    // 撮影モード
    err=dc1394_video_set_mode(camera, config.mode);
    DC1394_ERR_CLN_RTN(err,cleanup_and_exit(camera),"Could not set video mode\n");
    
    // format7 かどうかで分岐
    if(dc1394_is_video_mode_scalable(config.mode) == DC1394_TRUE) {
        err = dc1394_format7_set_roi(camera, config.mode, config.color_coding,  \
                                     DC1394_USE_MAX_AVAIL, config.left, config.top, config.width,          \
                                     config.height);
        DC1394_ERR_CLN_RTN(err,cleanup_and_exit(camera),"Could not set ROI\n");
    } else {
        err=dc1394_video_set_framerate(camera, config.frame_rate);
        DC1394_ERR_CLN_RTN(err,cleanup_and_exit(camera),"Could not set framerate\n");
    }
    
    err=dc1394_capture_setup(camera, config.num_dma_buffers, DC1394_CAPTURE_FLAGS_DEFAULT);
    if(err != DC1394_SUCCESS) {
        // 直前に不正終了していると，dc1394_capture_setup に失敗するので，一度リセットしてみる．
        DC1394_WRN(err, "Maybe an unclean shutdown was occured in the previous \
                   camera capturing.\nTring to reinitialize the bus...\n");
        dc1394_reset_bus(camera);
        // ちょっと待つ
        //        sleep(1);
        err=dc1394_capture_setup(camera, config.num_dma_buffers, DC1394_CAPTURE_FLAGS_DEFAULT);
    }
    if(err != DC1394_SUCCESS) {
        return err;
    }
    
    //    sleep(1);
    
    /// flush the ring buffer
    for(int i = 0; i < config.num_dma_buffers; i++) {
        dc1394video_frame_t * frame = NULL;
        if(dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_POLL, &frame) == DC1394_SUCCESS) {
            if(frame) {
                dc1394_capture_enqueue(camera, frame);
            }
        }
    }
    
    return DC1394_SUCCESS;
}

static uint64_t find_camera(dc1394_t * handle, uint64_t guid) {
    dc1394error_t err;
    dc1394camera_list_t * list;
    
    // カメラの一覧を得る
    err=dc1394_camera_enumerate(handle, &list);
    DC1394_ERR_RTN(err, "Failed to enumerate cameras");
    
    // カメラが見つからなかった？
    if(list->num == 0) {
        dc1394_log_error("No cameras found");
        return 1;
    }
    
    if(guid == 0) {
        // 特に指定がなければ，最初に見つかったカメラを使う
        guid = list->ids[0].guid;
        dc1394_camera_free_list(list);
        return guid;
    }
    
    // GUID が指定されている場合
    for(uint32_t i=0 ; i<list->num ; i++) {
        if(list->ids[i].guid == guid) {
            dc1394_camera_free_list(list);
            return guid;
        }
    }
    
    // 指定された GUID のカメラが見つからなかった
    dc1394_camera_free_list(list);
    dc1394_log_error("cannot find camera with GUID = %llx\n", guid);
    return 1;
}

DCam DCam_new(const CameraConfig config, const uint64_t guid) {
    dc1394error_t err;
    
    DCam dcam = (DCam)malloc(sizeof(struct DCam_impl));
    dcam->handle = dc1394_new();
    dcam->guid = find_camera(dcam->handle, guid);
    dcam->camera = dc1394_camera_new(dcam->handle, dcam->guid);
    dcam->frame = NULL;
    
    if(! dcam->camera) {
        dc1394_log_error("Failed to initialize camera with guid %llx", dcam->guid);
        return NULL;
    }
    
    err = setup_capture(dcam->camera, config);
    if(err != DC1394_SUCCESS) {
        dc1394_log_error("Cannot setup the capture. Check your camera and/or try \
                         with fewer dma_numbuffers\n");
        cleanup_and_exit(dcam->camera);
        return NULL;
    }
    
    return dcam;
}

void DCam_delete(DCam * dcam) {
    dc1394_capture_stop((*dcam)->camera);
    dc1394_camera_free((*dcam)->camera);
    dc1394_free((*dcam)->handle);
    free(*dcam);
    *dcam = NULL;
}

void DCam_start_capture(DCam dcam) {
    dc1394error_t err=dc1394_video_set_transmission(dcam->camera, DC1394_ON);
    DC1394_ERR_CLN(err,cleanup_and_exit(dcam->camera),"Could not start camera iso transmission\n");
}

void DCam_stop_capture(DCam dcam) {
    dc1394error_t err = dc1394_video_set_transmission(dcam->camera,DC1394_OFF);
    DC1394_ERR_CLN(err,cleanup_and_exit(dcam->camera),"Could not stop the camera");
}

const unsigned char * DCam_grab_and_lock(DCam dcam) {
    dc1394error_t err;
    err=dc1394_capture_dequeue(dcam->camera, DC1394_CAPTURE_POLICY_WAIT, &(dcam->frame));
    if(err != DC1394_SUCCESS) {
        dc1394_log_error("Could not capture a frame\n");
        cleanup_and_exit(dcam->camera);
    }
    
    return dcam->frame->image;
}

void DCam_unlock_grabbed(DCam dcam) {
    if(dcam->frame) {
        dc1394_capture_enqueue(dcam->camera, dcam->frame);
        dcam->frame = NULL;
    }
}

// is dropped frame ?
bool isDropFrame(const DCam _dcam, const CameraConfig& _config) {
    // cout << "_dcam->frame->frames_behind, _config.num_dma_buffers = "
    //   << _dcam->frame->frames_behind << ", " << _config.num_dma_buffers << endl;
    if (_dcam->frame->frames_behind < _config.num_dma_buffers) {
        return false;
    } else {
        return true;
    }
}

// カメラ画像をCV形式で取得
void captureCVImage(cv::Mat* const _image, const DCam _dcam){
    // キャプチャ = １フレーム分バッファをロック
    DCam_grab_and_lock(_dcam);
    
    // CV画像へ変換
    convertDC1394imageToCVimage(_image, _dcam->frame->image);
    
    // ロック解除
    DCam_unlock_grabbed(_dcam);
}
