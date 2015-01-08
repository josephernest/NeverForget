// compile with: cl neverforget.cpp

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comctl32.lib")

#ifndef UNICODE 
#define UNICODE
#endif 

#include <windows.h>
#include <commctrl.h>

#include <string>
#include <fstream>
#include <streambuf>
#include <vector>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK EditWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwrefData);

HWND hwndMain = NULL;
HWND hwndEdit = NULL;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
    const wchar_t CLASS_NAME[]  = L"Sample Window Class";
    WNDCLASS wc = { };
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    RECT rect;
    HWND hwndTaskbar = FindWindow(L"Shell_traywnd", NULL);
    GetWindowRect(hwndTaskbar, &rect);
    hwndMain = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_LAYERED, wc.lpszClassName, 0, WS_POPUP | WS_VISIBLE | WS_SYSMENU, rect.right - 300, 3 * rect.top / 4, 300, rect.top / 4, 0, 0, 0, 0);
    hwndEdit = CreateWindowEx(0, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL, 0, 0, 300, rect.top / 4, hwndMain, 0, (HINSTANCE) GetWindowLong(hwndMain, GWL_HINSTANCE), NULL);
    SetLayeredWindowAttributes(hwndMain, 0, 192, LWA_ALPHA);
    ShowWindow(hwndMain, nCmdShow);

    HWND hwndOwner = GetWindow(GetWindow(GetTopWindow(0), GW_HWNDLAST), GW_CHILD);   // makes the main window part of the desktop
    SetWindowLong(hwndMain, GWL_HWNDPARENT, (LONG) hwndOwner);      

    SetWindowSubclass(hwndEdit, (SUBCLASSPROC) EditWndProc, 0, 1);

    std::ifstream t("neverforget.txt", std::ios::in  | std::ios::binary);
    std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

    SendMessageA(hwndEdit, WM_SETTEXT, 0, (LPARAM) str.c_str()); 

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

void Serialize()
{
    int len = GetWindowTextLengthA(hwndEdit);
    std::vector<char> text(len+1);
    GetWindowTextA(hwndEdit, &text[0], len+1);
    std::ofstream file("neverforget.txt", std::ios::out  | std::ios::binary);
    file.write(&text[0], len);
    file.close();    
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_COMMAND:              
        {
            if (HIWORD(wParam) == EN_CHANGE)      // if textbox has changed
            { 
                Serialize();
            }
            return 0;
        }

        case WM_DESTROY:
        {
            //Serialize();
            PostQuitMessage(0);
            return 0;
        } 

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            HBRUSH hBrush = CreateSolidBrush(RGB(255,255,255));
            FillRect(hdc, &ps.rcPaint, hBrush);
            EndPaint(hwnd, &ps);
            return 0;            
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK EditWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwrefData)
{
    switch(message)
    {
        case WM_KEYDOWN:
        {
            if (wParam=='A' && (::GetKeyState(VK_CONTROL) & 0x8000)!=0)       // CTRL+A for "select all"
            {
                SendMessage(hwndEdit, EM_SETSEL, 0, -1);
                return 0;
            }
        }
    }
    return DefSubclassProc(hwnd, message, wParam, lParam);
}