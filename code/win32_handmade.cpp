#include <windows.h>
#include <xinput.h>
#include <stdint.h>
#define internal   static
#define  local_persist static 
#define global_variable  static 

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

struct win32_offscreen_buffer
{
  BITMAPINFO Info;
  void *Memory;
  int width;
  int height;
  int Pitch; 
  int byteperpixel;
};

global_variable bool running;
global_variable win32_offscreen_buffer GlobaBackbuffer;

struct win32_window_dimension
{
  int width;
  int height;
};

win32_window_dimension GetWindowDimension(HWND window)
{
  win32_window_dimension result;

  RECT clientrect;
  GetClientRect(window,&clientrect);
  result.width = clientrect.right - clientrect.left;
  result.height = clientrect.bottom - clientrect.top;

  return (result);
}

internal void
RenderWeirdGradient(win32_offscreen_buffer *buffer, int BlueOffset, int GreenOffset)
{
  int width = buffer->width;
  int height = buffer->height;
  uint8 *row = (uint8 *)buffer->Memory;

  for (int y = 0;
      y < buffer->height;
      ++y)
      {
        uint32 *Pixel = (uint32 *)row;
        for (int x = 0;
            x < buffer->width;
            ++x)
            {
              uint8 Blue = (x + BlueOffset);
              uint8 Green = (y + GreenOffset);

              *Pixel++ = ((Green << 8) | Blue);
            }
            row += buffer->Pitch; 
      }
}

internal void 
win32ResizeDIBsection(win32_offscreen_buffer *buffer , int width,int height)
{
  if (buffer->Memory)
  {
    VirtualFree(buffer->Memory,0,MEM_RELEASE);
  }

  buffer->width = width;
  buffer->height = height;
  buffer->byteperpixel = 4;

  buffer->Info.bmiHeader.biSize = sizeof(buffer->Info.bmiHeader);
  buffer->Info.bmiHeader.biWidth = buffer->width;
  buffer->Info.bmiHeader.biHeight = -buffer->height;
  buffer->Info.bmiHeader.biPlanes = 1;
  buffer->Info.bmiHeader.biBitCount = 32;
  buffer->Info.bmiHeader.biCompression = BI_RGB;

  int BitmapMemorysize = (buffer->width * buffer->height) * buffer->byteperpixel;
  buffer->Memory = VirtualAlloc(0,BitmapMemorysize,MEM_COMMIT,PAGE_READWRITE);

  buffer->Pitch = width * buffer->byteperpixel;
}

internal void
win32DisplayBufferIoWindow(HDC devicecontext,int windowwidth,int windowheight,
                        win32_offscreen_buffer buffer,
                        int x,int y, int width, int height)
{
  StretchDIBits(devicecontext,
                /*x,y,width,height,
                x,y,width,height,*/ 
                0,0,windowwidth,windowheight,
                0,0,buffer.width,buffer.height,
                buffer.Memory,
                &buffer.Info,
                DIB_RGB_COLORS,SRCCOPY);
}
LRESULT CALLBACK win32MainWindowCallback
  (HWND window, 
  UINT message, 
  WPARAM wParam, 
  LPARAM lParam)
  {
    LRESULT result = 0;
    switch(message)
    {
      case WM_ACTIVATE:
      {
        OutputDebugStringA("wm_activate");
        break;
      }
      case WM_CLOSE:
      {
        running = false;
      }
      case WM_SIZE:
      {
        break;
      }
      case WM_DESTROY:
      {
        running = false;
      }
      case WM_PAINT:
      {
        PAINTSTRUCT paint;
        HDC devicecontext = BeginPaint(window,&paint);
        int x = paint.rcPaint.left;
        int y = paint.rcPaint.top;
        int width = paint.rcPaint.right - paint.rcPaint.left;
        int height = paint.rcPaint.bottom - paint.rcPaint.top;

        win32_window_dimension Dimention = GetWindowDimension(window);
        win32DisplayBufferIoWindow(devicecontext,Dimention.width,Dimention.height,
                                    GlobaBackbuffer,x,y,width,height);
        EndPaint(window,&paint);
        break;
      }
      default:
      {
        OutputDebugStringA("wm_activate");
        result = DefWindowProcA(window,message,wParam,lParam);
        break;
      }
    }
    return result;
  }
int  CALLBACK WinMain(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPSTR     lpCmdLine,
  int       nShowCmd)
  {
    
    WNDCLASSA windowClass = {};

    win32ResizeDIBsection(&GlobaBackbuffer,1280,720);

    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = win32MainWindowCallback;
    windowClass.hInstance = hInstance;
    windowClass.lpszClassName = "HandmadeHeroWindowClass";

   if (RegisterClassA(&windowClass))
   {
      HWND Window = CreateWindowExA(
                  0,
                  windowClass.lpszClassName,
                  "Handmade Hero",
                  WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                  CW_USEDEFAULT,
                  CW_USEDEFAULT,
                  CW_USEDEFAULT,
                  CW_USEDEFAULT,
                  0,
                  0,
                  hInstance,
                  0);
      if (Window)
      {
        int Xoffset = 0;
        int Yoffset = 0;
        running = true;
        while(running)
        {
          MSG message;
          while(PeekMessageA(&message,0,0,0,PM_REMOVE))
          {
            if (message.message == WM_QUIT)
            {
              running = false;
            }
            TranslateMessage(&message);
            DispatchMessageA(&message);
          }
          RenderWeirdGradient(&GlobaBackbuffer,Xoffset,Yoffset);
          HDC devicecontext = GetDC(Window);
          win32_window_dimension Dimention = GetWindowDimension(Window);
          win32DisplayBufferIoWindow(devicecontext,Dimention.width,Dimention.height,
          GlobaBackbuffer,0,0,Dimention.width,Dimention.height);
          ReleaseDC(Window,devicecontext);
          ++Xoffset;
          Yoffset += 1;
        }
      }
      else
      {
        // TODO logging
      }
   }
   else
   {
        // TODO logging
   }
  
    return 0;
  }
