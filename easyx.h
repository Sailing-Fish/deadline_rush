// ============================================================
//  easyx.h — EasyX API 兼容层 for MinGW/g++
//  提供最常用的 EasyX 函数，底层用 Win32 GDI 实现
//  用法：#include "easyx.h" 即可，和真 EasyX 一样
// ============================================================
#pragma once

#ifndef UNICODE
#define UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <vector>
#include <cmath>

#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

// ==================== 全局状态 ====================
namespace easyx_impl {
    inline HWND    g_hwnd     = NULL;
    inline HDC     g_hdc      = NULL;
    inline HDC     g_memDC    = NULL;
    inline HBITMAP g_memBmp   = NULL;
    inline HBITMAP g_oldBmp   = NULL;
    inline int     g_width    = 640;
    inline int     g_height   = 480;
    inline bool    g_batch    = false;
    inline COLORREF g_fillColor  = RGB(255,255,255);
    inline COLORREF g_textColor  = RGB(0,0,0);
    inline COLORREF g_bkColor    = RGB(0,0,0);
    inline COLORREF g_lineColor  = RGB(255,255,255);
    inline int     g_textH    = 16;
    inline int     g_textW    = 0;
    inline bool    g_inited   = false;
    inline int     g_vpX      = 0;
    inline int     g_vpY      = 0;
    inline float   g_scale    = 1.0f;

    // 鼠标状态
    inline int     g_mouseX   = 0;
    inline int     g_mouseY   = 0;
    inline bool    g_mouseDown = false;
    inline bool    g_mouseClicked = false;

    // 键盘
    inline bool    g_keys[256] = {};

    LRESULT CALLBACK EasyXWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
        switch(msg) {
        case WM_LBUTTONDOWN: g_mouseDown=true; g_mouseClicked=true;
            g_mouseX=LOWORD(lp); g_mouseY=HIWORD(lp); break;
        case WM_LBUTTONUP:   g_mouseDown=false; break;
        case WM_MOUSEMOVE:   g_mouseX=LOWORD(lp); g_mouseY=HIWORD(lp); break;
        case WM_KEYDOWN:     g_keys[(int)wp]=true; break;
        case WM_KEYUP:       g_keys[(int)wp]=false; break;
        case WM_DESTROY:     PostQuitMessage(0); break;
        default: return DefWindowProc(hwnd,msg,wp,lp);
        }
        return 0;
    }
}

// ==================== EasyX API ====================

// 初始化图形窗口
inline void initgraph(int width, int height, int flag=0) {
    using namespace easyx_impl;
    g_width=width; g_height=height;

    WNDCLASSEXW wc={};
    wc.cbSize=sizeof(wc);
    wc.lpfnWndProc=EasyXWndProc;
    wc.hInstance=GetModuleHandleW(NULL);
    wc.hCursor=LoadCursorW(NULL,IDC_ARROW);
    wc.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName=L"EasyXWindow";
    RegisterClassExW(&wc);

    RECT wr={0,0,width,height};
    AdjustWindowRect(&wr,WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_THICKFRAME|WS_MAXIMIZEBOX,FALSE);

    int sx=(GetSystemMetrics(SM_CXSCREEN)-width)/2;
    int sy=(GetSystemMetrics(SM_CYSCREEN)-height)/2;

    g_hwnd=CreateWindowExW(0,L"EasyXWindow",L"EasyX",
        WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_THICKFRAME|WS_MAXIMIZEBOX,
        sx,sy,wr.right-wr.left,wr.bottom-wr.top,
        NULL,NULL,wc.hInstance,NULL);

    g_hdc=GetDC(g_hwnd);
    g_memDC=CreateCompatibleDC(g_hdc);
    g_memBmp=CreateCompatibleBitmap(g_hdc,width,height);
    g_oldBmp=(HBITMAP)SelectObject(g_memDC,g_memBmp);
    g_inited=true;

    ShowWindow(g_hwnd,SW_SHOW);
    UpdateWindow(g_hwnd);
}

// 关闭图形窗口
inline void closegraph() {
    using namespace easyx_impl;
    if(!g_inited) return;
    if(g_hdc){
        SelectObject(g_memDC,g_oldBmp);
        DeleteObject(g_memBmp);
        DeleteDC(g_memDC);
        ReleaseDC(g_hwnd,g_hdc);
    }
    DestroyWindow(g_hwnd);
    g_inited=false;
}

// 检查是否有未处理的消息
inline bool is_run() {
    MSG msg;
    while(PeekMessageW(&msg,NULL,0,0,PM_REMOVE)){
        if(msg.message==WM_QUIT) return false;
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return true;
}

// 清除屏幕
inline void cleardevice() {
    using namespace easyx_impl;
    RECT r={0,0,g_width,g_height};
    HBRUSH b=CreateSolidBrush(g_bkColor);
    FillRect(g_memDC,&r,b);
    DeleteObject(b);
}

// 设置背景色
inline void setbkcolor(COLORREF c) { easyx_impl::g_bkColor=c; }

// 设置填充颜色
inline void setfillcolor(COLORREF c) { easyx_impl::g_fillColor=c; }

// 设置线条颜色
inline void setlinecolor(COLORREF c) { easyx_impl::g_lineColor=c; }

// 设置文字颜色
inline void settextcolor(COLORREF c) { easyx_impl::g_textColor=c; }

// 设置文字样式
inline void settextstyle(int h, int w, const wchar_t* face) {
    easyx_impl::g_textH=h; easyx_impl::g_textW=w;
}

// 设置文字样式 (char*版本)
inline void settextstyle(int h, int w, const char* face) {
    easyx_impl::g_textH=h; easyx_impl::g_textW=w;
}

// 填充矩形
inline void fillrectangle(int x1, int y1, int x2, int y2) {
    using namespace easyx_impl;
    RECT r={x1,y1,x2,y2};
    HBRUSH b=CreateSolidBrush(g_fillColor);
    FillRect(g_memDC,&r,b);
    DeleteObject(b);
}

// 实心矩形（和fillrectangle一样）
inline void solidrectangle(int x1,int y1,int x2,int y2) {
    fillrectangle(x1,y1,x2,y2);
}

// 空心矩形
inline void rectangle(int x1,int y1,int x2,int y2) {
    using namespace easyx_impl;
    HPEN p=CreatePen(PS_SOLID,1,g_lineColor);
    SelectObject(g_memDC,p);
    HBRUSH nb=(HBRUSH)GetStockObject(NULL_BRUSH);
    SelectObject(g_memDC,nb);
    Rectangle(g_memDC,x1,y1,x2,y2);
    DeleteObject(p);
}

// 线段
inline void line(int x1,int y1,int x2,int y2) {
    using namespace easyx_impl;
    HPEN p=CreatePen(PS_SOLID,1,g_lineColor);
    SelectObject(g_memDC,p);
    MoveToEx(g_memDC,x1,y1,NULL); LineTo(g_memDC,x2,y2);
    DeleteObject(p);
}

// 文字输出 (宽字符)
inline void outtextxy(int x, int y, const wchar_t* text) {
    using namespace easyx_impl;
    // 负值=精确像素高度，避免逻辑单位导致的模糊
    int exactH=-g_textH;
    HFONT f=CreateFontW(exactH,g_textW,0,0,
        (g_textH>=20)?FW_BOLD:FW_NORMAL, FALSE,FALSE,FALSE,
        DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,DEFAULT_PITCH|FF_DONTCARE,
        L"Microsoft YaHei");
    if(!f) f=CreateFontW(exactH,g_textW,0,0,FW_NORMAL,FALSE,FALSE,FALSE,
        DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,DEFAULT_PITCH|FF_DONTCARE,NULL);
    SelectObject(g_memDC,f);
    SetTextColor(g_memDC,g_textColor);
    SetBkMode(g_memDC,TRANSPARENT);
    TextOutW(g_memDC,x,y,text,(int)wcslen(text));
    DeleteObject(f);
}

// 文字输出 (窄字符)
inline void outtextxy(int x, int y, const char* text) {
    int len=MultiByteToWideChar(CP_UTF8,0,text,-1,NULL,0);
    wchar_t* w=new wchar_t[len+1];
    MultiByteToWideChar(CP_UTF8,0,text,-1,w,len);
    outtextxy(x,y,w);
    delete[] w;
}

// 文字输出 (std::string)
inline void outtextxy(int x, int y, const std::string& s) { outtextxy(x,y,s.c_str()); }

inline int textwidth(const wchar_t* text) {
    using namespace easyx_impl;
    int exactH=-g_textH;
    HFONT f=CreateFontW(exactH,g_textW,0,0,
        (g_textH>=20)?FW_BOLD:FW_NORMAL, FALSE,FALSE,FALSE,
        DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,DEFAULT_PITCH|FF_DONTCARE,
        L"Microsoft YaHei");
    if(!f) return (int)wcslen(text)*g_textH/2;
    SelectObject(g_memDC,f);
    SIZE sz{};
    GetTextExtentPoint32W(g_memDC,text,(int)wcslen(text),&sz);
    DeleteObject(f);
    return sz.cx;
}

// 圆形
inline void fillcircle(int cx,int cy,int r) {
    using namespace easyx_impl;
    HPEN p=CreatePen(PS_SOLID,1,g_fillColor);
    HBRUSH b=CreateSolidBrush(g_fillColor);
    SelectObject(g_memDC,p); SelectObject(g_memDC,b);
    Ellipse(g_memDC,cx-r,cy-r,cx+r,cy+r);
    DeleteObject(p); DeleteObject(b);
}
inline void circle(int cx,int cy,int r) {
    using namespace easyx_impl;
    HPEN p=CreatePen(PS_SOLID,1,g_lineColor);
    HBRUSH b=(HBRUSH)GetStockObject(HOLLOW_BRUSH);
    SelectObject(g_memDC,p); SelectObject(g_memDC,b);
    Ellipse(g_memDC,cx-r,cy-r,cx+r,cy+r);
    DeleteObject(p);
}

// 清屏后批量绘制
inline void BeginBatchDraw() { easyx_impl::g_batch=true; }
inline void FlushBatchDraw() {
    using namespace easyx_impl;
    RECT rc;
    GetClientRect(g_hwnd,&rc);
    int cw=rc.right,ch=rc.bottom;
    
    // 计算缩放比例（保持宽高比）
    float scale=(float)std::min(cw/(float)g_width,ch/(float)g_height);
    g_scale=scale;
    int drawW=(int)(g_width*scale);
    int drawH=(int)(g_height*scale);
    int vx=(cw-drawW)/2;
    int vy=(ch-drawH)/2;
    g_vpX=vx; g_vpY=vy;
    
    // 填充边缘黑边
    HBRUSH brush=CreateSolidBrush(g_bkColor);
    if(vy>0){RECT t={0,0,cw,vy};FillRect(g_hdc,&t,brush);}
    if(ch>vy+drawH){RECT b={0,vy+drawH,cw,ch};FillRect(g_hdc,&b,brush);}
    if(vx>0){RECT l={0,vy,vx,vy+drawH};FillRect(g_hdc,&l,brush);}
    if(cw>vx+drawW){RECT r={vx+drawW,vy,cw,vy+drawH};FillRect(g_hdc,&r,brush);}
    DeleteObject(brush);
    
    // 缩放绘制
    if(scale==1.0f){
        BitBlt(g_hdc,vx,vy,drawW,drawH,g_memDC,0,0,SRCCOPY);
    }else{
        SetStretchBltMode(g_hdc,COLORONCOLOR);
        StretchBlt(g_hdc,vx,vy,drawW,drawH,g_memDC,0,0,g_width,g_height,SRCCOPY);
    }
}
inline void EndBatchDraw() {
    FlushBatchDraw();
    easyx_impl::g_batch=false;
}

// 获取鼠标状态
inline void GetMouseMsg() {} // no-op for PeekMouseMsg
inline bool MouseHit() { return easyx_impl::g_mouseClicked; }
inline int  GetMouseX() { easyx_impl::g_mouseClicked=false; return easyx_impl::g_mouseX; }
inline int  GetMouseY() { easyx_impl::g_mouseClicked=false; return easyx_impl::g_mouseY; }

// 键盘
inline bool GetAsyncKeyStateCompat(int vk) { return easyx_impl::g_keys[vk]; }
inline bool kbhit() { for(int i=0;i<256;i++) if(easyx_impl::g_keys[i]) return true; return false; }

// 窗口句柄
inline HWND GetHWnd() { return easyx_impl::g_hwnd; }
inline int  getwidth()  { return easyx_impl::g_width; }
inline int  getheight() { return easyx_impl::g_height; }

// 获取HDC（兼容）
inline HDC GetImageHDC() { return easyx_impl::g_memDC; }

// 延时
inline void Sleep_ms(int ms) { Sleep(ms); }

// 简化初始化：initgraph(width, height)
// flag: 可设为 EW_SHOWCONSOLE 等
#define EW_SHOWCONSOLE 1
#define EW_NOCLOSE     0