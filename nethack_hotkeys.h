#pragma once
#include <string>
#include <vector>

// 快捷键项结构
struct HotkeyItem {
    std::wstring key;          // 显示的键名
    std::wstring description;  // 描述
    std::wstring category;     // 分类
    int vkCode;                // 虚拟键码
    bool shift;                // 是否需要 Shift
    bool ctrl;                 // 是否需要 Ctrl
    bool alt;                  // 是否需要 Alt
};

// 分类结构
struct Category {
    std::wstring id;
    std::wstring name;
    bool enabled;
};

// 获取所有快捷键
const std::vector<HotkeyItem>& GetHotkeys();

// 获取所有分类
const std::vector<Category>& GetCategories();

// 根据分类获取快捷键
std::vector<HotkeyItem> GetHotkeysByCategory(const std::wstring& categoryId);

// 获取分类名称
std::wstring GetCategoryName(const std::wstring& categoryId);
