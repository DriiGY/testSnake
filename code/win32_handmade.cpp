#include "utils.cpp"
#include "math.cpp"

#include <windows.h>

#include <stdio.h>

/*char buffer[256];
                sprintf(buffer, "Cursor coordinates -> x:%i y: %i\n",(int)pt.x,(int)pt.y);
                OutputDebugStringA(buffer);*/
////////////////////////////
// Snake com menu, pacman ou tetris.

/*
SNAKE
TODOS that I know:

Create rectangle!
Make background black.
Square moves with arrows.
Spawn random square inside screen.
if snake touches food it disappears and snake gets bigger.
Snake passes window limi it spawns on the other side.
Snake moves automatically.HOW???????
Score of how many foods you ate.
If snake touches other part of body, game is over!

Amanha:
Refactor code: create GetDimensions function for client rect width and height.

Create Rectangle for snake.
Use keys to move snake.
Add flag exists to say square exists or not. And change pos of rectangle with this info.

*/

// Render for painting stuff. or paltform independent file.

/*
 Se criarmos um bitblit, o device context tem de ser o mesmo e a janela destination
e source tem de ser do mesmo tamanho. Para transferir dados entre DC de diferentes devices
posso converter o bitmap para um Device independent bitmap com GetDIBits().
 Para fazer display deste DIB no segundo device posso usar SetDIBits or StretchDIBits.
-- No meu caso, os devices seriam os mesmos.
Posso usar stretchBlt criando um bitmap para o device context atual naquele momento
usando CreateDIBSection que cria um Device Independent Bitmap.

 Vou usar do handmadehero stretchDiBits que cria um DIB quando a janela muda de tamanho por exemplo,  so tenho que fazer stretch.
Cria um bitmapinfo e poe-o em memoria para ser usado sempre que ser preciso quando 
usarmos stretchDiBits.


------
Basicamente, virtualAlloc 'e o equivalente a malloc no CRT.
Ou seja, vou allocar um array dynamico ao tamanho da minha imagem que, neste caso,
'e a minha client area. Precisa de ser dinamico porque muda sempre que faco resize.

Eu estava como o GetMessage no while e aquilo nunca saia do loop  porque (ver diferenca entre
GetMessage e PeekMessage) nunca sai.
Se usar com o if fica bem a GetMessage.
Posso usar o PeekMessage no while porque nao esta sempre a procura de mensagens logo conseguem sair do loop e correr o resto do codigo.
Testar/Debug deste codigo mas com o peekmessage a ver se corre o resto do codigo a seguir ao loop while


------


GetWindowDC vs GetDC:

GetWindowDC() function retrieves the device context (DC) for the entire window, including title bar, menus, and scroll bars. A window device context permits painting anywhere in a window, because the origin of the device context is the upper-left corner of the window instead of the client area.

GetDC() function retrieves a handle to a display device context for the client area of a specified window or for the entire screen.

-------------------
After resizing window need to re scale so my mouse is in sync:
https://gamedev.stackexchange.com/questions/101092/problem-why-the-mouse-cursor-position-is-totally-wrong-after-resizing-the-wind




*/


struct Win32_Screen_Buffer {
    uint32 BytesPerPixel;
    BITMAPINFO BitMapInfo;
    void *BitMapInfo_Memory;  // void pointer LPVOID
    uint32 Stride;
    uint32 Width;
    uint32 Height;
    
};

struct Win32_Screen_Dimensions {
    int Height;
    int Width;
};


// Creating global_variable just for testing
global_variable Win32_Screen_Buffer GlobalScreenBuffer;
global_variable POINT pt;
global_variable bool32 GlobalRunning;
global_variable f32 scale = 0.01f;



inline f32
Calculate_Aspect_Multiplier()
{
    f32 AspectMultiplier = (f32)GlobalScreenBuffer.Height;
    if((f32)GlobalScreenBuffer.Width/(f32)GlobalScreenBuffer.Height<1.77f)
    {
        AspectMultiplier = (f32)GlobalScreenBuffer.Width / 1.77f;
    }
    return AspectMultiplier;
}

internal Win32_Screen_Dimensions
Win32_Get_Screen_Dimensions(HWND Window)
{
    Win32_Screen_Dimensions Result;
    
    RECT ClientRect;
    GetClientRect(Window, &ClientRect); 
    Result.Height = ClientRect.bottom - ClientRect.top;
    Result.Width = ClientRect.right - ClientRect.left;
    return Result;
}

internal void
PaintScreen(Win32_Screen_Buffer *ScreenBuffer, int RedOffset, int GreenOffset, int BlueOffset)
{
    // TODO : Add green to create any color for screen paint. Get offsets
    // if 0 screen is black
    
    
    // Draw pixel colors
    // nao faz sentido no update porque estou a fazer update e depois a mudar a cor.
    // criar a section com a cor e depois posso fazer update
    // BitMapInfo_Memory 'e um array que representa um 2D array da minha client area.
    
    // PAINT TO SCREEN A COLOR FOR STARTERS
    
    // Point to first bye of row
    uint8 *Row = (uint8 *)ScreenBuffer->BitMapInfo_Memory;
    for(int y=0;y<ScreenBuffer->Height;y++)
    {
        uint32 *Pixel =(uint32 *) Row; // Point to first pixel of Row 
        for(int x=0;x<ScreenBuffer->Width;x++)
        {
            //P  R  G  B
            //00 00 00 ff  - Blue screen(P for padding)
            //00 ff 00 00  - Red screen(P for padding)
            uint8 Red = (x+RedOffset);
            //uint8 Green = (y+GreenOffset); // let;s make it go horizontally
            uint8 Blue = (y+BlueOffset);
            *Pixel++ = (Red<<16)|Blue;
            //*(Pixel++) = (Red<<16)|(Green<<8)|Blue;
        }
        Row+=ScreenBuffer->Stride; // we use first byte so we can skip one row by mult by stride.
    }
    
}

v2 player; // this shoudl not be a global_variable
v2 half_size = { 1, 1};
uint32 red = 0x00ff0000;
uint32 blue = 0x000000ff;
uint32 color = red;
internal void
DrawRectangle(Win32_Screen_Buffer *ScreenBuffer, int x70, int y70, int x71, int y71, uint32 *color)
{
    // Get width and height
    // Coordinates need to be inside screen
    
    // check coordinates for square
    /*
    x0 = clamp(0, x0, ScreenBuffer->Width);
    y0 = clamp(0, y0, ScreenBuffer->Height);
    x1 = clamp(0, x1, ScreenBuffer->Width);
    y1 = clamp(0, y1, ScreenBuffer->Height);*/
    
    f32 aspect_multiplier = Calculate_Aspect_Multiplier();
    
    half_size.x *= aspect_multiplier * scale;
    half_size.y *= aspect_multiplier * scale;
    
    player.x *= aspect_multiplier * scale;
    player.y *= aspect_multiplier * scale;
    
    player.x += (f32)GlobalScreenBuffer.Width * .5f;
    player.y += (f32)GlobalScreenBuffer.Height * .5f;
    
    //converts units to pixel and calls draw_rect_in_pixels
    int x0 = (int)(player.x - half_size.x);
    int y0 = (int)(player.y - half_size.y);
    int x1 = (int)(player.x + half_size.x);
    int y1 = (int)(player.y + half_size.y);
    
    
    x0 = clamp(0, x0, ScreenBuffer->Width);
    y0 = clamp(0, y0, ScreenBuffer->Height);
    x1 = clamp(0, x1, ScreenBuffer->Width);
    y1 = clamp(0, y1, ScreenBuffer->Height);
    
    if(*color == red)
    {
        *color = blue;
    }else 
    {
        *color = red;
    }
    // WTF AM I A DOING !!! I DONT GET ANYTHIGNG A GJS JGS AMOFSMDOIGNDSOI
    // change this shit ! COLOR STUFF AND WORK RATIO
    // create repo for this shit
    
    uint8 *Row = (uint8 *)ScreenBuffer->BitMapInfo_Memory + x0*ScreenBuffer->BytesPerPixel + y0*ScreenBuffer->Stride;
    
    for(int y=y0;y<y1;y++)
    {
        uint32 *Pixel =(uint32 *) (Row );
        
        for(int x=x0;x<x1;x++)
        {
            *Pixel++ = *color;
        }
        Row+=ScreenBuffer->Stride; // we use first byte so we can skip one row by mult by stride.
    }
    
}

internal void
Win32_CreateDibSection(Win32_Screen_Buffer *ScreenBuffer, int PrevWidth, int  PrevHeight)
{
    //Create bitmapinfo and header to allocate memory for screen needed
    if(ScreenBuffer->BitMapInfo_Memory)
    {
        VirtualFree(&ScreenBuffer->BitMapInfo_Memory, 0, MEM_RELEASE);
    }
    
    ScreenBuffer->Width = PrevWidth;
    ScreenBuffer->Height = PrevHeight;
    
    ScreenBuffer->BitMapInfo.bmiHeader.biSize = sizeof(ScreenBuffer->BitMapInfo.bmiHeader) ;
    ScreenBuffer->BitMapInfo.bmiHeader.biWidth = PrevWidth ;
    ScreenBuffer->BitMapInfo.bmiHeader.biHeight = -PrevHeight;
    ScreenBuffer->BitMapInfo.bmiHeader.biPlanes = 1;
    ScreenBuffer->BitMapInfo.bmiHeader.biBitCount = 32; // 1 byte per RGB + 1 for Padding
    ScreenBuffer->BitMapInfo.bmiHeader.biCompression = BI_RGB;
    ScreenBuffer->BytesPerPixel = 4;
    uint32 BitMapInfo_Size = (ScreenBuffer->Width*ScreenBuffer->Height)*ScreenBuffer->BytesPerPixel;
    /* STRIDE
 In an uncompressed bitmap, the stride is the number of bytes needed to
 go from the start of one row of pixels to the start of the next row.
*/
    ScreenBuffer->Stride = ScreenBuffer->Width * ScreenBuffer->BytesPerPixel; 
    ScreenBuffer->BitMapInfo_Memory = VirtualAlloc(NULL, BitMapInfo_Size, MEM_RESERVE|MEM_COMMIT , PAGE_READWRITE); 
    
    //PaintScreen(&GlobalScreenBuffer, 1, 1, 1);
    //DrawRectangle(&GlobalScreenBuffer, 400, 400, 405, 405);
}
internal void 
Win32_UpdateWindow(Win32_Screen_Buffer *ScreenBuffer, HDC DeviceContext, int LastWidth, int LastHeight)
{
    // vamos dizer que o PrevWidth e Height vai ser quando fez resize e o current
    // quando fez paint so para TEST.
    
    StretchDIBits(DeviceContext,
                  0, 0, LastWidth, LastHeight,
                  0, 0,ScreenBuffer-> Width, ScreenBuffer->Height,
                  ScreenBuffer->BitMapInfo_Memory,
                  &(ScreenBuffer->BitMapInfo),
                  DIB_RGB_COLORS,
                  SRCCOPY
                  );
    
    
}


internal LRESULT  
Wndproc(
        HWND Window,
        UINT Message,
        WPARAM WParam,
        LPARAM LParam
        )
{
    LRESULT Result;
    switch(Message)
    {
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            bool32 is_down = ((LParam & (1 << 31)) == 0);
            char buffer[256];
            sprintf(buffer, "is_down: %d\n", is_down);
            OutputDebugStringA(buffer);
        }break;
        case WM_CLOSE:
        case WM_DESTROY:
        {
            // TODO(casey): Handle this with a message to the user?
            GlobalRunning = false;
        } break;
        case WM_SIZE:
        {
            RECT ClientRect;
            GetClientRect(Window, &ClientRect); 
            int Height = ClientRect.bottom - ClientRect.top;
            int Width = ClientRect.right - ClientRect.left;
            //Win32_CreateDibSection(&GlobalScreenBuffer, Width, Height);
            
        }break;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC DeviceContext = BeginPaint(Window, &ps);
            int Width = ps.rcPaint.right - ps.rcPaint.left;
            int Height =  ps.rcPaint.top - ps.rcPaint.bottom;
            Win32_UpdateWindow(&GlobalScreenBuffer, DeviceContext, Width, Height);
            EndPaint(Window, &ps);
        }break;
        case WM_LBUTTONDOWN:
        {
            pt.x = LOWORD(LParam);
            pt.y = HIWORD(LParam);
            
            
        }break;
        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam); 
        }break;
        
    }
    return Result;
}


int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{
    WNDCLASSA wc  = {};
    //Win32ResizeDIBSection(&GlobalScreenBuffer, 1280, 720);
    wc.style = CS_VREDRAW | CS_HREDRAW| CS_OWNDC;
    wc.lpfnWndProc = &Wndproc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = Instance;
    wc.hIcon = NULL;
    wc.hCursor = NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "WindowClass";
    if(!RegisterClassA(&wc))
    {
        OutputDebugStringA("Windowclass was not able to be registered!\n");
    }else
    {
        HWND Window = CreateWindowA(wc.lpszClassName,
                                    "New Window",
                                    WS_OVERLAPPEDWINDOW| WS_VISIBLE,
                                    CW_USEDEFAULT ,
                                    CW_USEDEFAULT ,
                                    CW_USEDEFAULT ,
                                    CW_USEDEFAULT ,
                                    NULL,
                                    NULL,
                                    Instance,
                                    NULL
                                    );
        if(Window)
        {
            Win32_Screen_Dimensions Dimensions = Win32_Get_Screen_Dimensions(Window);
            char buffer[256];
            sprintf(buffer, "Aspect ratio:%.6f, width:%i, height:%i \n",(f32)((f32)Dimensions.Width/(f32)Dimensions.Height), Dimensions.Width, Dimensions.Height);
            OutputDebugStringA(buffer);
            bool32 RectFlag = false;
            GlobalRunning = true;
            
            //bool32 is_down = ((Message.lParam & (1 << 31)) == 0);
            Win32_CreateDibSection(&GlobalScreenBuffer, Dimensions.Width, Dimensions.Height);
            PaintScreen(&GlobalScreenBuffer, 1, 1, 1);
            while(GlobalRunning)
            {
                MSG Message;
                //Win32_Screen_Buffer  = {};
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    if(Message.message == WM_QUIT)
                    {
                        GlobalRunning = false;
                    }
                    
                    
                    TranslateMessage(&Message); 
                    DispatchMessage(&Message); 
                    
                }
                HDC DC = GetDC(Window);
                Win32_Screen_Dimensions Dimensions = Win32_Get_Screen_Dimensions(Window);
                
                //DrawRectangle(&GlobalScreenBuffer, 0,0,0,0);
                DrawRectangle(&GlobalScreenBuffer, (int)pt.x, (int)pt.y, (int)(pt.x+5), (int)(pt.y+5), &color);
                Win32_UpdateWindow(&GlobalScreenBuffer, DC, Dimensions.Width, Dimensions.Height);
                ReleaseDC(Window, DC);
            }
            
        }
    }
    return(0);
}