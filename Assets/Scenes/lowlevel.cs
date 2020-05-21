using UnityEngine;
using System.Collections;
using System;
using System.Runtime.InteropServices;

public class lowlevel : MonoBehaviour
{
    [DllImport("CVCamera_GL_LowLevel")]
    private static extern IntPtr GetCamera(int device,int width, int height);
    [DllImport("CVCamera_GL_LowLevel")]
    private static extern void ReleaseCamera(IntPtr ptr);
    [DllImport("CVCamera_GL_LowLevel")]
    private static extern int SetCameraTexturePtr(IntPtr ptr, IntPtr texture);
    [DllImport("CVCamera_GL_LowLevel")]
    private static extern IntPtr GetRenderEventFunc();

    private IntPtr camera_ = IntPtr.Zero;

    Texture2D tex;

    [SerializeField]
    int width = 1920;
    [SerializeField]
    int height = 1080;

    void Start()
    {
        camera_ = GetCamera(0,width,height);

        tex = new Texture2D(
            width,
            height,
            TextureFormat.RGBA32,
            false, /* mipmap */
            true /* linear */);

        GetComponent<MeshRenderer>().material.mainTexture = tex;

        SetCameraTexturePtr(camera_, tex.GetNativeTexturePtr());
        StartCoroutine(OnRender());
    }

    void OnDestroy()
    {
        ReleaseCamera(camera_);
    }
    IEnumerator OnRender()
    {
        for (; ; )
        {
            yield return new WaitForEndOfFrame();
            GL.IssuePluginEvent(GetRenderEventFunc(), 0);
        }
    }
}
