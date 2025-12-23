#include <windows.h>
#include <stdint.h>

#define local_persist static 
#define global_var static 
#define internal static 



global_var bool Running;

global_var BITMAPINFO BitMapInfo;
global_var void *BitmapMemory;
global_var int BitmapWidth;
global_var int BitmapHeight;
global_var int BytesPerPixel = 4;

internal void RenderTest(int XOffset, int YOffset){
    int Pitch = BitmapWidth*BytesPerPixel;
    uint8_t *Row = (uint8_t *)BitmapMemory;

    for(int y = 0; y < BitmapHeight; y++){
        uint32_t *Pixel = (uint32_t *)Row;
        for(int x = 0; x < BitmapWidth; x++){
            
            uint8_t Blue = (x + XOffset);
            uint8_t Green = (y + YOffset);
            *Pixel++ = ((Green << 8)  | Blue);
        }

        Row += Pitch;
    }
}

internal void ResizeDIBSection(int Width, int Height){

    if(BitmapMemory){
        VirtualFree(BitmapMemory, 0, MEM_RELEASE);
    }

    BitmapWidth = Width;
    BitmapHeight = Height;

    BitMapInfo.bmiHeader.biSize = sizeof(BitMapInfo.bmiHeader);
    BitMapInfo.bmiHeader.biWidth = BitmapWidth;
    BitMapInfo.bmiHeader.biHeight = -BitmapHeight;
    BitMapInfo.bmiHeader.biPlanes = 1;
    BitMapInfo.bmiHeader.biBitCount = 32;
    BitMapInfo.bmiHeader.biCompression = BI_RGB;

    int BitmapMemorySize = (Width * Height) * BytesPerPixel;

    BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

}

internal void WindowUpdate(HDC DeviceContext, RECT WindowRect, int X, int Y, int Width, int Height){

    int WindowWidth = WindowRect.right - WindowRect.left;
    int WindowHeight = WindowRect.bottom - WindowRect.top;

    StretchDIBits(DeviceContext, 
            0, 0, BitmapWidth, BitmapHeight,
            0, 0, WindowWidth, WindowHeight,
            BitmapMemory,
            &BitMapInfo,
            DIB_RGB_COLORS, SRCCOPY);

}

LRESULT CALLBACK MainWindowCallBack(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;

    switch (Message)
    {
        case WM_SIZE:
        {
            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            int Height = ClientRect.bottom - ClientRect.top;
            int Width = ClientRect.right - ClientRect.left;
            ResizeDIBSection(Width, Height);
            OutputDebugStringA("WM_SIZE\n");
        }break;
        
        case WM_DESTROY:
        {
            OutputDebugStringA("WM_DESTROY\n");
            Running = false;
        }break;
        
        case WM_CLOSE:
        {
            Running = false;
            OutputDebugStringA("WM_CLOSE\n");
        }break;

        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;

        case WM_PAINT:{

            PAINTSTRUCT Paint;

            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;
            WindowUpdate(DeviceContext, ClientRect,X, Y, Width, Height);
            EndPaint(Window, &Paint);

        }

        default:
        {
//          OutputDebugStringA("default\n");
            Result = DefWindowProc(Window, Message, WParam, LParam);
        }break;
    }
    return(Result);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{

    WNDCLASS WindowClass = {};

    WindowClass.style = CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = MainWindowCallBack;
    WindowClass.hInstance = hInstance;
    WindowClass.lpszClassName = "GameTestWindowClass";

    if(RegisterClass(&WindowClass)){
        HWND Window = CreateWindowEx(
                        0,
                        WindowClass.lpszClassName,
                        "Game Test",
                        WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        0,
                        0,
                        hInstance,
                        0);
        if(Window){
            Running = true;
            int XOffset = 0;
            int YOffset = 0;
            
            while(Running){
                MSG Message;

                while(PeekMessage(&Message, 0, 0,0, PM_REMOVE)){
                    if(Message.message == WM_QUIT){
                        Running = false;
                    }
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                RenderTest(XOffset, YOffset);
                HDC DeviceContext = GetDC(Window); 
                RECT ClientRect;
                GetClientRect(Window, &ClientRect);
                int WindowWidth = ClientRect.right - ClientRect.left;
                int WindowHeight = ClientRect.bottom - ClientRect.top;
                WindowUpdate(DeviceContext, ClientRect,0, 0, WindowWidth, WindowHeight);
                ReleaseDC(Window, DeviceContext);
                XOffset++;
            }
        }
        else{
            return 1;
        }
        return 0;
    }
}