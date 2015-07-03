//
//  myDc1394.h
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/08/03.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//

#ifndef __cameraBase03__myDc1394__
#define __cameraBase03__myDc1394__

#include <iostream>
#include <dc1394/dc1394.h>
#include "myOpenCV.h"

/// カメラ
struct DCam_impl {
    uint64_t guid;				// カメラID
    dc1394_t * handle;			// バス全体のハンドル
    dc1394camera_t * camera;		// 使用しているカメラのハンドル
    dc1394video_frame_t * frame;	// ロック中のフレーム
};
typedef struct DCam_impl *DCam;

/// カメラの設定
typedef struct {
    int need_reset;					/// カメラを初期化する？
    dc1394speed_t speed;				/// バスの帯域幅
    dc1394video_mode_t mode;			/// 撮影モード
    int num_dma_buffers;				/// DMA バッファの数（32程度まで．多すぎると dc1394_capture_setup に失敗する）
    dc1394framerate_t frame_rate;		/// (for non-format7) フレームレート
    dc1394color_coding_t color_coding;/// (for format7) 色変換の指示
    int width; 						/// (for format7) ROI
    int height; 						/// (for format7) ROI
    int top;							/// (for format7) ROI
    int left;							/// (for format7) ROI
} CameraConfig;

const CameraConfig config_grasshopper_uxga = {
    1,                           // カメラを初期化する？
    DC1394_ISO_SPEED_800,        // バスの帯域幅
    DC1394_VIDEO_MODE_FORMAT7_0, // 撮影モード
    16, // DMA バッファの数（32程度まで．多すぎると dc1394_capture_setup に失敗する）
    DC1394_FRAMERATE_30,         // (for non-format7) フレームレート ... ダミー
    DC1394_COLOR_CODING_RAW8,    // (for format7) 色変換の指示
    1600,                        // (for format7) ROI
    1200,                        // (for format7) ROI
    12,                          // (for format7) ROI
    12,                          // (for format7) ROI
};

const CameraConfig config_dragonfly_vga = {
    1,                           // カメラを初期化する？
    DC1394_ISO_SPEED_400,        // バスの帯域幅
    DC1394_VIDEO_MODE_640x480_MONO8, // 撮影モード
    16,                           // DMA バッファの数（32程度まで．多すぎると dc1394_capture_setup に失敗する）
    DC1394_FRAMERATE_30,         // (for non-format7) フレームレート
    DC1394_COLOR_CODING_RAW8,    // (for format7) 色変換の指示 ... ダミー
    640,                         // (for format7) ROI ... ダミー
    480,                         // (for format7) ROI ... ダミー
    0,                           // (for format7) ROI ... ダミー
    0,                           // (for format7) ROI ... ダミー
};

const CameraConfig config_dragonfly_vga_color = {
    1,                           // カメラを初期化する？
    DC1394_ISO_SPEED_400,        // バスの帯域幅
    DC1394_VIDEO_MODE_640x480_RGB8, // 撮影モード
    16,                           // DMA バッファの数（32程度まで．多すぎると dc1394_capture_setup に失敗する）
    // DC1394_FRAMERATE_15,         // (for non-format7) フレームレート
    DC1394_FRAMERATE_15,         // (for non-format7) フレームレート
    DC1394_COLOR_CODING_RAW8,    // (for format7) 色変換の指示 ... ダミー
    640,                         // (for format7) ROI ... ダミー
    480,                         // (for format7) ROI ... ダミー
    0,                           // (for format7) ROI ... ダミー
    0,                           // (for format7) ROI ... ダミー
};

///////////////////////////////  convert method ///////////////////////////////
void convertDC1394imageToCVimage(cv::Mat* const _imageCV, const uchar* const _imageDC);
void convertCVimageToDC1394image(uchar* const _imageDC, const cv::Mat& _imageCV);

///////////////////////////////  print method ///////////////////////////////
void printAllCameraFeatures(dc1394camera_t& _camera);

///////////////////////////////  other method ///////////////////////////////
static void cleanup_and_exit(dc1394camera_t *camera);
static dc1394error_t setup_capture(dc1394camera_t * camera, const CameraConfig config);
static uint64_t find_camera(dc1394_t * handle, uint64_t guid);
DCam DCam_new(const CameraConfig config, const uint64_t guid);
void DCam_delete(DCam * dcam);
void DCam_start_capture(DCam dcam);
void DCam_stop_capture(DCam dcam);
const unsigned char * DCam_grab_and_lock(DCam dcam);
void DCam_unlock_grabbed(DCam dcam);
bool isDropFrame(const DCam _dcam, const CameraConfig& _config);
void captureCVImage(cv::Mat* const _image, const DCam _dcam);

#endif /* defined(__cameraBase03__myDc1394__) */
