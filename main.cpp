#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <tlhelp32.h>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include "nethack_hotkeys.h"
#include "config.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "kernel32.lib")

static HFONT g_hFont = NULL;

void CreateGlobalFont() {
    NONCLIENTMETRICSW ncm;
    ncm.cbSize = sizeof(NONCLIENTMETRICSW);
    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICSW), &ncm, 0);
    
    g_hFont = CreateFontW(
        ncm.lfMessageFont.lfHeight,
        ncm.lfMessageFont.lfWidth,
        ncm.lfMessageFont.lfEscapement,
        ncm.lfMessageFont.lfOrientation,
        ncm.lfMessageFont.lfWeight,
        ncm.lfMessageFont.lfItalic,
        ncm.lfMessageFont.lfUnderline,
        ncm.lfMessageFont.lfStrikeOut,
        ncm.lfMessageFont.lfCharSet,
        ncm.lfMessageFont.lfOutPrecision,
        ncm.lfMessageFont.lfClipPrecision,
        ncm.lfMessageFont.lfQuality,
        ncm.lfMessageFont.lfPitchAndFamily,
        L"Segoe UI"
    );
    
    if (!g_hFont) {
        g_hFont = CreateFontW(
            -12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
        );
    }
}

void DeleteGlobalFont() {
    if (g_hFont) {
        DeleteObject(g_hFont);
        g_hFont = NULL;
    }
}

struct WindowInfo {
    HWND hWnd;
    std::wstring title;
};

struct FindWindowData {
    DWORD processId;
    HWND hWnd;
};

struct EnumWindowsData {
    std::vector<WindowInfo>* windows;
};

BOOL CALLBACK FindWindowByProcessIdCallback(HWND hWnd, LPARAM lParam) {
    FindWindowData* data = (FindWindowData*)lParam;
    DWORD pid = 0;
    GetWindowThreadProcessId(hWnd, &pid);
    if (pid == data->processId && IsWindowVisible(hWnd) && GetWindow(hWnd, GW_OWNER) == NULL) {
        data->hWnd = hWnd;
        return FALSE;
    }
    return TRUE;
}

HWND FindWindowByProcessId(DWORD processId) {
    FindWindowData data;
    data.processId = processId;
    data.hWnd = NULL;
    EnumWindows(FindWindowByProcessIdCallback, (LPARAM)&data);
    return data.hWnd;
}

BOOL CALLBACK EnumWindowsCallback(HWND hWnd, LPARAM lParam) {
    EnumWindowsData* data = (EnumWindowsData*)lParam;
    
    if (IsWindowVisible(hWnd) && GetWindow(hWnd, GW_OWNER) == NULL) {
        wchar_t title[512] = {0};
        GetWindowTextW(hWnd, title, 512);
        
        if (wcslen(title) > 0) {
            data->windows->push_back({hWnd, title});
        }
    }
    return TRUE;
}

static HWND g_selectedWindow = NULL;
BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam);

LRESULT CALLBACK SelectWindowWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static HWND hList = NULL;
    
    switch (message) {
    case WM_CREATE: {
        g_selectedWindow = NULL;
        
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        std::vector<WindowInfo>* pWindows = (std::vector<WindowInfo>*)pCreate->lpCreateParams;
        
        HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
        
        hList = CreateWindowExW(0, L"LISTBOX", L"",
            WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
            10, 10, 360, 180,
            hWnd, (HMENU)1001, hInst, NULL);
        
        for (size_t i = 0; i < pWindows->size(); i++) {
            int idx = (int)SendMessageW(hList, LB_ADDSTRING, 0, (LPARAM)(*pWindows)[i].title.c_str());
            SendMessageW(hList, LB_SETITEMDATA, (WPARAM)idx, (LPARAM)(*pWindows)[i].hWnd);
        }
        
        if (!pWindows->empty()) {
            SendMessageW(hList, LB_SETCURSEL, (WPARAM)0, 0);
        }
        
        CreateWindowExW(0, L"BUTTON", L"确定",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            100, 200, 90, 28,
            hWnd, (HMENU)IDOK, hInst, NULL);
        
        CreateWindowExW(0, L"BUTTON", L"取消",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            200, 200, 90, 28,
            hWnd, (HMENU)IDCANCEL, hInst, NULL);
        
        EnumChildWindows(hWnd, EnumChildProc, 0);
        return 0;
    }
    
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        int wmEvent = HIWORD(wParam);
        
        if (wmId == IDOK) {
            int sel = (int)SendMessageW(hList, LB_GETCURSEL, 0, 0);
            if (sel != LB_ERR) {
                g_selectedWindow = (HWND)SendMessageW(hList, LB_GETITEMDATA, (WPARAM)sel, 0);
            }
            DestroyWindow(hWnd);
            return 0;
        }
        
        if (wmId == IDCANCEL) {
            DestroyWindow(hWnd);
            return 0;
        }
        
        if (wmId == 1001 && wmEvent == LBN_DBLCLK) {
            int sel = (int)SendMessageW(hList, LB_GETCURSEL, 0, 0);
            if (sel != LB_ERR) {
                g_selectedWindow = (HWND)SendMessageW(hList, LB_GETITEMDATA, (WPARAM)sel, 0);
            }
            DestroyWindow(hWnd);
            return 0;
        }
        break;
    }
    
    case WM_DESTROY:
        return 0;
    
    case WM_CLOSE:
        DestroyWindow(hWnd);
        return 0;
    }
    
    return DefWindowProcW(hWnd, message, wParam, lParam);
}

HWND ShowWindowSelectionDialog(HWND hParent, HINSTANCE hInst) {
    std::vector<WindowInfo> windows;
    EnumWindowsData data;
    data.windows = &windows;
    EnumWindows(EnumWindowsCallback, (LPARAM)&data);
    
    if (windows.empty()) {
        MessageBoxW(hParent, L"未找到任何窗口", L"提示", MB_OK);
        return NULL;
    }
    
    std::sort(windows.begin(), windows.end(), [](const WindowInfo& a, const WindowInfo& b) {
        return a.title < b.title;
    });
    
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = SelectWindowWndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"SelectWindowDialogClass";
    
    RegisterClassExW(&wc);
    
    HWND hDlg = CreateWindowExW(0, L"SelectWindowDialogClass", L"选择目标窗口",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 280,
        hParent, NULL, hInst, (LPVOID)&windows);
    
    if (!hDlg) {
        wchar_t errMsg[256];
        wsprintfW(errMsg, L"无法创建对话框，错误码: %d", GetLastError());
        MessageBoxW(hParent, errMsg, L"错误", MB_OK);
        return NULL;
    }
    
    ShowWindow(hDlg, SW_SHOW);
    UpdateWindow(hDlg);
    
    MSG msg;
    while (IsWindow(hDlg) && GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    
    HWND result = g_selectedWindow;
    g_selectedWindow = NULL;
    return result;
}

#define WINDOW_WIDTH 570
#define WINDOW_HEIGHT 750

#define ID_BTN_SELECT_WINDOW 1001
#define ID_EDIT_SEARCH 1003
#define ID_BTN_CLEAR_SEARCH 1004
#define ID_HOTKEY_BTN_START 2000
#define ID_CATEGORY_RADIO_START 3000
#define ID_STATIC_NO_MATCH 4000

HINSTANCE g_hInst = NULL;
HWND g_hMainWnd = NULL;
HWND g_hEditSearch = NULL;
HWND g_hStaticTarget = NULL;
HWND g_hTargetWnd = NULL;
std::wstring g_targetWindowTitle = L"";
std::wstring g_searchQuery = L"";
int g_currentTabIndex = 0;

void RefreshHotkeyList();
void SendKeyToWindow(const HotkeyItem& item);
std::wstring EscapeButtonText(const std::wstring& text);

BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam) {
    SendMessageW(hWnd, WM_SETFONT, (WPARAM)g_hFont, TRUE);
    return TRUE;
}

std::wstring EscapeButtonText(const std::wstring& text) {
    std::wstring result;
    for (wchar_t c : text) {
        if (c == L'&') {
            result += L"&&";
        } else {
            result += c;
        }
    }
    return result;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
        g_hInst = ((LPCREATESTRUCT)lParam)->hInstance;
        g_hMainWnd = hWnd;
        
        
        // 标题
        CreateWindowExW(0, L"STATIC", L"NetHack 快捷键助手",
            WS_VISIBLE | WS_CHILD | SS_LEFT,
            15, 10, 220, 30, hWnd, NULL, g_hInst, NULL);
        
        // 选择窗口按钮
        CreateWindowExW(0, L"BUTTON", L"选择窗口",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT,
            420, 10, 100, 28, hWnd, (HMENU)ID_BTN_SELECT_WINDOW, g_hInst, NULL);
        
        // 目标窗口显示
        g_hStaticTarget = CreateWindowExW(0, L"STATIC", L"未选择窗口",
            WS_VISIBLE | WS_CHILD | SS_LEFT,
            15, 45, 500, 20, hWnd, NULL, g_hInst, NULL);
        
        // 搜索标签
        CreateWindowExW(0, L"STATIC", L"搜索:",
            WS_VISIBLE | WS_CHILD | SS_LEFT,
            15, 75, 50, 20, hWnd, NULL, g_hInst, NULL);
        
        // 搜索框
        g_hEditSearch = CreateWindowExW(0, L"EDIT", L"",
            WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL | WS_BORDER,
            65, 72, 380, 26, hWnd, (HMENU)ID_EDIT_SEARCH, g_hInst, NULL);
        
        // 清除按钮
        CreateWindowExW(0, L"BUTTON", L"X",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT,
            455, 72, 30, 26, hWnd, (HMENU)ID_BTN_CLEAR_SEARCH, g_hInst, NULL);
        
        // 分类标签
        const wchar_t* tabs[] = {L"移动", L"攻击", L"物品", L"额外", L"状态", L"特殊", L"系统"};
        int tabX = 15;
        for (int i = 0; i < 7; i++) {
            CreateWindowExW(0, L"BUTTON", tabs[i],
                WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | BS_FLAT,
                tabX, 105, 60, 24,
                hWnd, (HMENU)(ID_CATEGORY_RADIO_START + i), g_hInst, NULL);
            tabX += 70;
        }
        CheckRadioButton(hWnd, ID_CATEGORY_RADIO_START, ID_CATEGORY_RADIO_START + 6, ID_CATEGORY_RADIO_START);
        
        RefreshHotkeyList();
        
        EnumChildWindows(hWnd, EnumChildProc, 0);
        break;
    }
    
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        
        if (wmId == ID_BTN_SELECT_WINDOW) {
            g_hTargetWnd = ShowWindowSelectionDialog(hWnd, g_hInst);
            
            if (g_hTargetWnd) {
                wchar_t title[512] = {0};
                GetWindowTextW(g_hTargetWnd, title, 512);
                g_targetWindowTitle = title;
                SetWindowTextW(g_hStaticTarget, title);
            } else {
                SetWindowTextW(g_hStaticTarget, L"未选择窗口");
            }
            break;
        }
        
        if (wmId == ID_BTN_CLEAR_SEARCH) {
            SetWindowTextW(g_hEditSearch, L"");
            g_searchQuery = L"";
            RefreshHotkeyList();
            break;
        }
        
        if (wmId == ID_EDIT_SEARCH) {
            wchar_t search[256] = {0};
            GetWindowTextW(g_hEditSearch, search, 256);
            g_searchQuery = search;
            RefreshHotkeyList();
            break;
        }
        
        if (wmId >= ID_CATEGORY_RADIO_START && wmId < ID_CATEGORY_RADIO_START + 7) {
            g_currentTabIndex = wmId - ID_CATEGORY_RADIO_START;
            RefreshHotkeyList();
            break;
        }
        
        if (wmId >= ID_HOTKEY_BTN_START && wmId < ID_HOTKEY_BTN_START + 500) {
            const auto& hotkeys = GetHotkeys();
            int index = wmId - ID_HOTKEY_BTN_START;
            
            // 按钮ID直接对应快捷键数组的索引
            if (index >= 0 && index < (int)hotkeys.size()) {
                SendKeyToWindow(hotkeys[index]);
            }
            break;
        }
        
        break;
    }
    
    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam;
        HWND hStatic = (HWND)lParam;
        
        SetBkColor(hdc, RGB(240, 240, 240));
        SetTextColor(hdc, RGB(0, 0, 0));
        
        return (LRESULT)GetStockObject(WHITE_BRUSH);
    }
    break;
    
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
        
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void RefreshHotkeyList() {
    HWND hChild = GetWindow(g_hMainWnd, GW_CHILD);
    while (hChild) {
        HWND hNext = GetWindow(hChild, GW_HWNDNEXT);
        int id = GetWindowLong(hChild, GWL_ID);
        // 只删除快捷键按钮和"未找到匹配"静态控件
        if (id >= ID_HOTKEY_BTN_START && id < ID_HOTKEY_BTN_START + 500) {
            DestroyWindow(hChild);
        } else if (id == ID_STATIC_NO_MATCH) {
            DestroyWindow(hChild);
        }
        hChild = hNext;
    }
    
    const auto& hotkeys = GetHotkeys();
    int y = 155;
    int count = 0;
    int col = 0; // 0 = 左列, 1 = 右列
    int btnWidth = 260; // 两列宽度
    
    for (size_t i = 0; i < hotkeys.size(); i++) {
        bool show = false;
        const auto& cats = GetCategories();
        if (g_currentTabIndex < (int)cats.size()) {
            show = (hotkeys[i].category == cats[g_currentTabIndex].id);
        }
        
        if (show && (g_searchQuery.empty() || 
            hotkeys[i].description.find(g_searchQuery) != std::wstring::npos ||
            hotkeys[i].key.find(g_searchQuery) != std::wstring::npos)) {
            
            std::wstring text = L"[" + EscapeButtonText(hotkeys[i].key) + L"] " + EscapeButtonText(hotkeys[i].description);
            HWND hBtn = CreateWindowExW(0, L"BUTTON", text.c_str(),
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT,
                10+col * (btnWidth + 10), y, btnWidth, 30,
                g_hMainWnd, (HMENU)(ID_HOTKEY_BTN_START + i), g_hInst, NULL);
            SendMessageW(hBtn, WM_SETFONT, (WPARAM)g_hFont, TRUE);
            
            count++;
            // 交替列
            if (col == 0) {
                col = 1;
            } else {
                col = 0;
                y += 35;
            }
            if (y > WINDOW_HEIGHT - 20) break;
        }
    }
    
    if (count == 0) {
        HWND hStatic = CreateWindowExW(0, L"STATIC", 
            g_searchQuery.empty() ? L"没有数据" : L"未找到匹配",
            WS_VISIBLE | WS_CHILD | SS_CENTER,
            15, y, WINDOW_WIDTH - 35, 20,
            g_hMainWnd, (HMENU)ID_STATIC_NO_MATCH, g_hInst, NULL);
        SendMessageW(hStatic, WM_SETFONT, (WPARAM)g_hFont, TRUE);
    }
}

void SendKeyToWindow(const HotkeyItem& item) {
    if (!g_hTargetWnd || !IsWindow(g_hTargetWnd)) {
        g_hTargetWnd = FindWindowW(NULL, L"NetHack");
        if (!g_hTargetWnd) {
            MessageBoxW(g_hMainWnd, L"请先选择 NetHack 窗口", L"提示", MB_OK);
            return;
        }
    }
    
    SetForegroundWindow(g_hTargetWnd);
    Sleep(50); // 给窗口切换一些时间
    
    INPUT inputs[10] = {0};
    int n = 0;
    
    // 按下修饰键（带扫描码）
    if (item.shift) {
        inputs[n].type = INPUT_KEYBOARD;
        inputs[n].ki.wVk = VK_SHIFT;
        inputs[n].ki.wScan = MapVirtualKeyW(VK_SHIFT, MAPVK_VK_TO_VSC);
        inputs[n].ki.dwFlags = 0;
        n++;
    }
    if (item.ctrl) {
        inputs[n].type = INPUT_KEYBOARD;
        inputs[n].ki.wVk = VK_CONTROL;
        inputs[n].ki.wScan = MapVirtualKeyW(VK_CONTROL, MAPVK_VK_TO_VSC);
        inputs[n].ki.dwFlags = 0;
        n++;
    }
    if (item.alt) {
        inputs[n].type = INPUT_KEYBOARD;
        inputs[n].ki.wVk = VK_MENU;
        inputs[n].ki.wScan = MapVirtualKeyW(VK_MENU, MAPVK_VK_TO_VSC);
        inputs[n].ki.dwFlags = 0;
        n++;
    }
    
    // 按下主按键（带扫描码）
    WORD scanCode = MapVirtualKeyW(item.vkCode, MAPVK_VK_TO_VSC);
    
    inputs[n].type = INPUT_KEYBOARD; 
    inputs[n].ki.wVk = (WORD)item.vkCode;
    inputs[n].ki.wScan = scanCode;
    inputs[n].ki.dwFlags = 0;
    n++;
    
    // 释放主按键
    inputs[n].type = INPUT_KEYBOARD; 
    inputs[n].ki.wVk = (WORD)item.vkCode;
    inputs[n].ki.wScan = scanCode;
    inputs[n].ki.dwFlags = KEYEVENTF_KEYUP;
    n++;
    
    // 释放修饰键
    if (item.alt) {
        inputs[n].type = INPUT_KEYBOARD;
        inputs[n].ki.wVk = VK_MENU;
        inputs[n].ki.wScan = MapVirtualKeyW(VK_MENU, MAPVK_VK_TO_VSC);
        inputs[n].ki.dwFlags = KEYEVENTF_KEYUP;
        n++;
    }
    if (item.ctrl) {
        inputs[n].type = INPUT_KEYBOARD;
        inputs[n].ki.wVk = VK_CONTROL;
        inputs[n].ki.wScan = MapVirtualKeyW(VK_CONTROL, MAPVK_VK_TO_VSC);
        inputs[n].ki.dwFlags = KEYEVENTF_KEYUP;
        n++;
    }
    if (item.shift) {
        inputs[n].type = INPUT_KEYBOARD;
        inputs[n].ki.wVk = VK_SHIFT;
        inputs[n].ki.wScan = MapVirtualKeyW(VK_SHIFT, MAPVK_VK_TO_VSC);
        inputs[n].ki.dwFlags = KEYEVENTF_KEYUP;
        n++;
    }
    
    SendInput(n, inputs, sizeof(INPUT));
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    CreateGlobalFont();
    
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"NetHackHelper";
    
    // 加载图标
    wc.hIcon = (HICON)LoadImageW(hInstance, L"IDI_ICON1", IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
    wc.hIconSm = (HICON)LoadImageW(hInstance, L"IDI_ICON1", IMAGE_ICON, 16, 16, LR_DEFAULTSIZE);
    
    // 如果图标加载失败，使用默认图标
    if (!wc.hIcon) {
        wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    }
    if (!wc.hIconSm) {
        wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    }
    
    RegisterClassExW(&wc);
    
    g_hInst = hInstance;
    g_hMainWnd = CreateWindowExW(0, L"NetHackHelper", L"Nethack Helper",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, hInstance, NULL);
    
    if (!g_hMainWnd) return 1;
    
    ShowWindow(g_hMainWnd, nCmdShow);
    UpdateWindow(g_hMainWnd);
    
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    
    DeleteGlobalFont();
    return (int)msg.wParam;
}
