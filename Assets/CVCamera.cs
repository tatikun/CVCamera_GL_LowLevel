using UnityEngine;
using System;
using System.Runtime.InteropServices;

public class CVCamera : MonoBehaviour
{
    [DllImport("DLL_CV")]
    private static extern IntPtr getCamera();
    [DllImport("DLL_CV")]
    private static extern void getCameraTexture(IntPtr camera, IntPtr data, int width, int height);

    private IntPtr camera_;
    private Texture2D texture_;
    private Color32[] pixels_;
    private GCHandle pixels_handle_;
    private IntPtr pixels_ptr_;
    void Start()
    {
        camera_ = getCamera(); //NativePlugin
        texture_ = new Texture2D(1920, 1080, TextureFormat.RGBA32, false);
        pixels_ = texture_.GetPixels32();
        pixels_handle_ = GCHandle.Alloc(pixels_, GCHandleType.Pinned);
        pixels_ptr_ = pixels_handle_.AddrOfPinnedObject();
        GetComponent<MeshRenderer>().material.mainTexture = texture_;
    }
    void Update()
    {
        getCameraTexture(camera_, pixels_ptr_, texture_.width, texture_.height);//NativePlugin
        texture_.SetPixels32(pixels_);
        texture_.Apply();
    }
}
