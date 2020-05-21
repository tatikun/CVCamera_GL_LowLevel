#include "pch.h"
#include <thread>

#include <opencv2/opencv.hpp>
#include <GL/glew.h>
#include <GL/freeglut.h>

// バージョン名の取得
#define CV_VERSION_STR CVAUX_STR(CV_MAJOR_VERSION) CVAUX_STR(CV_MINOR_VERSION) CVAUX_STR(CV_SUBMINOR_VERSION)

#pragma comment(lib,"opencv_world" CV_VERSION_STR ".lib")
#include "IUnityInterface.h"
#include "IUnityGraphics.h"

class Camera
{
private:
    cv::VideoCapture camera_;
    int width_, height_;
    cv::Mat image_;
    GLuint texture_ = 0;

    std::thread thread_;
    std::mutex mutex_;
    bool isRunning_ = false;

public:
    Camera(int device, int width, int height)
        : camera_(device)
        , width_(width)
        , height_(height)
    {
        camera_ = cv::VideoCapture(device,cv::CAP_DSHOW);
        //カメラの設定
        cv::Size captureSize(width, height);
        camera_.set(cv::CAP_PROP_FRAME_WIDTH, captureSize.width);
        camera_.set(cv::CAP_PROP_FRAME_HEIGHT, captureSize.height);
        camera_.set(cv::CAP_PROP_FPS, 30);
        //camera_.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G')); //圧縮設定
        StartCapture();
    }

    ~Camera()
    {
        StopCapture();
        camera_.release();
    }

private:
    void StartCapture()
    {
        thread_ = std::thread([this] {
            isRunning_ = true;
            while (isRunning_ && camera_.isOpened()) {
                cv::Mat bgr_image;
                camera_ >> bgr_image;
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    cv::flip(bgr_image, image_,0);
                }
            }
            });
    }

    void StopCapture()
    {
        isRunning_ = false;
        if (thread_.joinable()) {
            thread_.join();
        }
    }

public:
    void Update()
    {
        if (texture_ == 0 || image_.empty()) return;

        std::lock_guard<std::mutex> lock(mutex_);
        glBindTexture(GL_TEXTURE_2D, texture_);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image_.cols, image_.rows, GL_BGR, GL_UNSIGNED_BYTE, image_.data);
    }

    int GetWidth() const
    {
        return width_;
    }

    int GetHeight() const
    {
        return height_;
    }

    void SetTexturePtr(void* ptr)
    {
        texture_ = (GLuint)(size_t)(ptr);
    }
};

namespace
{
    Camera* g_camera = nullptr;
}

extern "C"
{
    // Unity 側で GL.IssuePlugin(この関数のポインタ, eventId) を呼ぶとレンダリングスレッドから呼ばれる
    void OnRenderEvent(int eventId)
    {
        if (g_camera) g_camera->Update();
    }

    // GL.IssuePlugin で登録するコールバック関数のポインタを返す
    UNITY_INTERFACE_EXPORT UnityRenderingEvent GetRenderEventFunc()
    {
        return OnRenderEvent;
    }

    //カメラクラス生成
    UNITY_INTERFACE_EXPORT void* GetCamera(int device,int width,int height)
    {
        g_camera = new Camera(device, width, height);
        return g_camera;
    }

    //カメラリソース解放
    UNITY_INTERFACE_EXPORT void ReleaseCamera(void* ptr)
    {
        auto camera = reinterpret_cast<Camera*>(ptr);
        if (g_camera == camera) g_camera = nullptr;
        delete camera;
    }

    //Unityのテクスチャ情報をカメラクラスへ与える
    UNITY_INTERFACE_EXPORT void SetCameraTexturePtr(void* ptr, void* texture)
    {
        auto camera = reinterpret_cast<Camera*>(ptr);
        camera->SetTexturePtr(texture);
    }
}