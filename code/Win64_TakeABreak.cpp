


#include <windows.h>
#include <string>
#include <iostream>

#define DEBUGGING_MODE


struct FrameEvents
{
    int cursorX, cursorY;
    bool mouseClick;
    bool hoverTray;

};

struct MainMemory
{

    HWND windowHandle;
    HWND consoleHandle;
    HWND desktopHandle;
    HINSTANCE exeHandle;
    NOTIFYICONDATA iconData;
    HANDLE bigIcon;
    HANDLE mediumIcon;
    HANDLE smallIcon;
    HMENU trayMenu;

    FrameEvents events;

    char exeFilePath[MAX_PATH];
    int exeFilePathSize = MAX_PATH;
    int exeFolderPathSize = 0;

    unsigned int alarmTimeInMinutes;

    bool running;

    const char* exeName = "TakeABreak";
};
const unsigned int TRAY_MESSAGE = (WM_USER + 1);

void Message(std::string message)
{
    std::cout << message << std::endl;
}

void Assert(bool test, std::string message, unsigned int severity, HWND consoleHandle)
{
#ifdef DEBUGGING_MODE
    if (!test)
    {
        ShowWindow(consoleHandle, SW_SHOW);
        std::cout << message.c_str() << std::endl;
        int x;
        std::cin >> x;
    }
#endif

}


LRESULT CALLBACK MessageCallback(HWND hwnd, UINT msg, WPARAM wP, LPARAM lP)
{
    MainMemory* mainMemory = (MainMemory*)GetWindowLongPtr(hwnd, 0);
    switch (msg)
    {
        case WM_CREATE:
        {
            CREATESTRUCTW* cs = (CREATESTRUCTW*)lP;
            SetWindowLongPtrW(hwnd, 0, (LONG_PTR)cs->lpCreateParams);
        }
        break;
        case WM_MENUCOMMAND:
        {
            Message("command");
        }
        break;
        case WM_CLOSE:
        case WM_DESTROY:
        {
            //Message("CLOSE");
            mainMemory->running = false;
        }
        break;
        case WM_MOUSEHOVER:
        {
            //Message("MOUSEHOVER");

        }
        break;
        case WM_MOUSEMOVE:
        {
            //Message("MOUSEMOVE");
        }
        break;
        case TRAY_MESSAGE:
        {
            //Message("TRAY_MESSAGE");


            switch (lP)
            {

                case WM_MENUCHAR:
                {
                    Message("MENUCHAR");
                }
                break;
                case WM_MOUSEMOVE:
                {
                    Message("MOUSEMOVE");
                }
                break;
                case WM_MOUSEHOVER:
                {
                    Message("MOUSEHOVER");
                    NOTIFYICONDATA* iconData = &mainMemory->iconData;
                    iconData->uFlags = NIF_STATE;
                    // Shell_NotifyIcon(NIM_MODIFY, &mainMemory->iconData);
                }
                break;
                case WM_LBUTTONDOWN:
                {
                    Message("MOUSE LEFT");

                    POINT cursorPoint;
                    GetCursorPos(&cursorPoint);
                    BOOL value = TrackPopupMenuEx(mainMemory->trayMenu, TPM_CENTERALIGN | TPM_BOTTOMALIGN | TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_VERNEGANIMATION, cursorPoint.x, cursorPoint.y, mainMemory->windowHandle, 0);
                    Message(std::to_string((UINT)value));



                    //DrawMenuBar(mainMemory->windowHandle);
                }
                break;
                case WM_RBUTTONDOWN:
                {
                    Message("MOUSE RIGHT");
                }
                break;
            }

        }
        break;
        default:
        {
            return DefWindowProc(hwnd, msg, wP, lP);
        }

    }
    return 0;

}



void InitIcon(MainMemory* mainMemory)
{
    TCHAR tip[128] = TEXT("TakeABreakTaskbar");
    TCHAR info[200] = TEXT("Take A Break has been Initialized.\nSee you when you need a break.");
    TCHAR infoTitle[48] = TEXT("TakeABreak");


    mainMemory->iconData.cbSize = NOTIFYICONDATAA_V3_SIZE;
    mainMemory->iconData.hWnd = mainMemory->windowHandle;
    //mainMemory->iconData.uID = 1; //if not using GUID, then windows uses the windowHandle plus uID to identify the icon(an app can have many icons)
    mainMemory->iconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    mainMemory->iconData.uCallbackMessage = TRAY_MESSAGE;
    mainMemory->iconData.hIcon = (HICON)mainMemory->bigIcon;
    //mainMemory->iconData.dwState = NIS_SHAREDICON;
    //mainMemory->iconData.dwStateMask = NIS_SHAREDICON;
    mainMemory->iconData.uVersion = NOTIFYICON_VERSION; // windows 2000 and up
    mainMemory->iconData.dwInfoFlags = NIIF_USER | NIIF_NOSOUND; //| NIIF_LARGE_ICON;
    //mainMemory->iconData.hBalloonIcon = (HICON)mainMemory->bigIcon; //ONLY FOR WINDOWS VISTA+, and alters shell_notifyIcon functionality

    strncpy_s(mainMemory->iconData.szTip, tip, sizeof(tip));
    strncpy_s(mainMemory->iconData.szInfo, info, sizeof(info));
    strncpy_s(mainMemory->iconData.szInfoTitle, infoTitle, sizeof(infoTitle));

    Shell_NotifyIcon(NIM_ADD, &mainMemory->iconData);

    mainMemory->trayMenu = CreatePopupMenu();
    AppendMenu(mainMemory->trayMenu, MF_ENABLED | MFT_STRING, 0, "CloseProcess");
    AppendMenu(mainMemory->trayMenu, MF_ENABLED | MFT_STRING, 0, "OpenWindow");
    AppendMenu(mainMemory->trayMenu, MF_ENABLED | MFT_STRING, 0, "CloseTrayMenu");
    //enable tray animation
    BOOL message = TRUE;
    BOOL error = SystemParametersInfo(SPI_SETMENUANIMATION, 0, &message, 0);
    BOOL error2 = SystemParametersInfo(SPI_SETMENUFADE, 0, &message, 0);
}
void CatChar(char* sourceA, int sizeA,
             char* sourceB, int sizeB,
             char* dest, int* destSize)
{

    *destSize = sizeA + sizeB;
    for (int i = 0; i < sizeA; i++)
    {
        *dest++ = *sourceA++;
    }
    for (int i = 0; i < sizeB; i++)
    {
        *dest++ = *sourceB++;
    }
}

int CharStarSize(char* source)
{
    int count = 0;
    while (*source++)
    {
        count++;
    }
    return count;
}

void SetHandle(MainMemory* mainMemory, char* file, HANDLE* handle)
{
    char* iconFileName = file;
    char iconFullFileName[MAX_PATH] = { 0 };
    int ignored;
    CatChar(mainMemory->exeFilePath, mainMemory->exeFolderPathSize, iconFileName, CharStarSize(iconFileName), iconFullFileName, &ignored);
    *handle = LoadImage(nullptr, iconFullFileName, IMAGE_ICON, 0, 0, LR_SHARED | LR_LOADFROMFILE); //without LR_DEFAULTSIZE  and the size specified, loadimage will use the resources dimensions
}


int main(int consoleNum, char** consoleChar)
{
    //init mainmemory
    MainMemory* mainMemory = new MainMemory();
    mainMemory->running = true;
    mainMemory->exeHandle = GetModuleHandle(nullptr);
    mainMemory->desktopHandle = GetDesktopWindow();
    mainMemory->consoleHandle = GetConsoleWindow();

    //ShowWindow(mainMemory->consoleHandle, SW_HIDE);

    //get exe folder path
    DWORD guessedSize = MAX_PATH;
    DWORD actualSize = GetModuleFileName(nullptr, mainMemory->exeFilePath, guessedSize);
    Assert(actualSize < guessedSize, "folder path is too big", 1, mainMemory->consoleHandle);
    mainMemory->exeFilePathSize = actualSize;
    mainMemory->exeFolderPathSize = actualSize;

    char* scan = &mainMemory->exeFilePath[mainMemory->exeFilePathSize - 1];
    while (*scan-- != '\\')
    {
        mainMemory->exeFolderPathSize--;
    }

    SetHandle(mainMemory, "data\\TAB.ico", &mainMemory->smallIcon);
    SetHandle(mainMemory, "data\\TAB.ico", &mainMemory->mediumIcon);
    SetHandle(mainMemory, "data\\TAB.ico", &mainMemory->bigIcon);
    //calculate alarmtime
    if (consoleNum == 2)
    {
        char* scan = *consoleChar;
        while (*scan++ != ' ')
        {}
        unsigned int sum = 0;
        while (*scan++ && *scan != ' ')
        {
            sum = (*scan) + sum * 10;
        }
        mainMemory->alarmTimeInMinutes = sum;
    }
    else
    {
        mainMemory->alarmTimeInMinutes = 20;
    }

    //create window
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = MessageCallback;
    wc.hInstance = mainMemory->exeHandle;
    wc.lpszClassName = mainMemory->exeName;
    wc.lpszMenuName = mainMemory->exeName;
    wc.style = CS_DBLCLKS;
    wc.hIcon = (HICON)mainMemory->mediumIcon;
    wc.hIconSm = (HICON)mainMemory->mediumIcon;
    wc.cbWndExtra = sizeof(void*);
    //TODO make cursor art
    wc.hCursor = (HCURSOR)mainMemory->mediumIcon; //LoadCursor(mainMemory->exeHandle, IDC_ARROW);

    ATOM error = RegisterClassEx(&wc);
    Assert(error > 0, "could not register windowclass", 1, mainMemory->consoleHandle);
    mainMemory->windowHandle = CreateWindowEx(0,
                               mainMemory->exeName,
                               mainMemory->exeName,
                               WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                               CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                               mainMemory->desktopHandle,
                               nullptr,
                               mainMemory->exeHandle,
                               mainMemory);
    ShowWindow(mainMemory->windowHandle, SW_SHOW);




    //create tray icon
    InitIcon(mainMemory);


    TCHAR zeroString[64] = " ";

    //strncpy_s(mainMemory->iconData.szTip, zeroString, 128);
    //strncpy_s(mainMemory->iconData.szInfo, zeroString, 200);
    //strncpy_s(mainMemory->iconData.szInfoTitle, zeroString, 48);
    //Shell_NotifyIcon(NIM_MODIFY, &mainMemory->iconData);


    MSG msg;
    while(mainMemory->running && GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Shell_NotifyIcon(NIM_DELETE, &mainMemory->iconData);
    ShowWindow(mainMemory->consoleHandle, SW_HIDE);
    return 0;
}

