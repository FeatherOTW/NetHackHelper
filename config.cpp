#include "config.h"
#include <windows.h>
#include <shlobj.h>
#include <fstream>
#include <sstream>

Config& Config::Instance() {
    static Config instance;
    return instance;
}

Config::Config() : m_windowX(100), m_windowY(100) {
    m_targetWindowTitle = L"NetHack";
    // 默认启用所有分类
    m_enabledCategories.insert(L"movement");
    m_enabledCategories.insert(L"attack");
    m_enabledCategories.insert(L"items");
    m_enabledCategories.insert(L"inventory");
    m_enabledCategories.insert(L"status");
    m_enabledCategories.insert(L"system");
    m_enabledCategories.insert(L"special");
    
    Load();
}

Config::~Config() {
    Save();
}

std::wstring GetConfigPath() {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    std::wstring::size_type pos = std::wstring(path).find_last_of(L"\\/");
    return std::wstring(path).substr(0, pos) + L"\\nethack_helper.ini";
}

void Config::Load() {
    std::wstring configPath = GetConfigPath();
    
    // 读取目标窗口标题
    wchar_t buffer[256] = {0};
    GetPrivateProfileStringW(L"Settings", L"TargetWindow", L"NetHack", buffer, 256, configPath.c_str());
    m_targetWindowTitle = buffer;
    
    // 读取窗口位置
    m_windowX = GetPrivateProfileIntW(L"Settings", L"WindowX", 100, configPath.c_str());
    m_windowY = GetPrivateProfileIntW(L"Settings", L"WindowY", 100, configPath.c_str());
    
    // 读取启用的分类
    m_enabledCategories.clear();
    const wchar_t* defaultCategories[] = {
        L"movement", L"attack", L"items", L"inventory", 
        L"status", L"system", L"special"
    };
    
    for (const auto& cat : defaultCategories) {
        int enabled = GetPrivateProfileIntW(L"Categories", cat, 1, configPath.c_str());
        if (enabled) {
            m_enabledCategories.insert(cat);
        }
    }
    
    // 读取常用快捷键索引
    m_favoriteHotkeys.clear();
    wchar_t favBuffer[512] = {0};
    GetPrivateProfileStringW(L"Favorites", L"Indices", L"", favBuffer, 512, configPath.c_str());
    if (favBuffer[0] != L'\0') {
        std::wstring favStr = favBuffer;
        std::wstring::size_type start = 0;
        std::wstring::size_type comma = favStr.find(L',');
        while (comma != std::wstring::npos) {
            std::wstring token = favStr.substr(start, comma - start);
            m_favoriteHotkeys.insert(_wtoi(token.c_str()));
            start = comma + 1;
            comma = favStr.find(L',', start);
        }
        // 最后一个
        std::wstring token = favStr.substr(start);
        if (!token.empty()) {
            m_favoriteHotkeys.insert(_wtoi(token.c_str()));
        }
    }
}

void Config::Save() {
    std::wstring configPath = GetConfigPath();
    
    // 保存目标窗口标题
    WritePrivateProfileStringW(L"Settings", L"TargetWindow", 
        m_targetWindowTitle.c_str(), configPath.c_str());
    
    // 保存窗口位置
    wchar_t buffer[32];
    wsprintfW(buffer, L"%d", m_windowX);
    WritePrivateProfileStringW(L"Settings", L"WindowX", buffer, configPath.c_str());
    wsprintfW(buffer, L"%d", m_windowY);
    WritePrivateProfileStringW(L"Settings", L"WindowY", buffer, configPath.c_str());
    
    // 保存启用的分类
    const wchar_t* allCategories[] = {
        L"movement", L"attack", L"items", L"inventory", 
        L"status", L"system", L"special"
    };
    
    for (const auto& cat : allCategories) {
        std::wstring value = m_enabledCategories.count(cat) ? L"1" : L"0";
        WritePrivateProfileStringW(L"Categories", cat, value.c_str(), configPath.c_str());
    }
    
    // 保存常用快捷键索引
    wchar_t favBuffer[512] = {0};
    std::wstring favStr;
    for (int idx : m_favoriteHotkeys) {
        favStr += std::to_wstring(idx) + L",";
    }
    if (!favStr.empty()) {
        favStr.pop_back();  // 移除最后一个逗号
    }
    WritePrivateProfileStringW(L"Favorites", L"Indices", favStr.c_str(), configPath.c_str());
}

bool Config::IsCategoryEnabled(const std::wstring& categoryId) const {
    return m_enabledCategories.count(categoryId) > 0;
}

void Config::SetCategoryEnabled(const std::wstring& categoryId, bool enabled) {
    if (enabled) {
        m_enabledCategories.insert(categoryId);
    } else {
        m_enabledCategories.erase(categoryId);
    }
}

std::wstring Config::GetTargetWindowTitle() const {
    return m_targetWindowTitle;
}

void Config::SetTargetWindowTitle(const std::wstring& title) {
    m_targetWindowTitle = title;
}

int Config::GetWindowX() const {
    return m_windowX;
}

int Config::GetWindowY() const {
    return m_windowY;
}

void Config::SetWindowPos(int x, int y) {
    m_windowX = x;
    m_windowY = y;
}

const std::set<int>& Config::GetFavoriteHotkeys() const {
    return m_favoriteHotkeys;
}

void Config::SetFavoriteHotkeys(const std::set<int>& favorites) {
    m_favoriteHotkeys = favorites;
}

void Config::AddFavoriteHotkey(int index) {
    m_favoriteHotkeys.insert(index);
}

void Config::RemoveFavoriteHotkey(int index) {
    m_favoriteHotkeys.erase(index);
}

bool Config::IsFavoriteHotkey(int index) const {
    return m_favoriteHotkeys.count(index) > 0;
}
