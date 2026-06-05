#pragma once
#include <string>
#include <vector>
#include <set>

// 配置管理类
class Config {
public:
    static Config& Instance();
    
    // 加载/保存配置
    void Load();
    void Save();
    
    // 分类开关
    bool IsCategoryEnabled(const std::wstring& categoryId) const;
    void SetCategoryEnabled(const std::wstring& categoryId, bool enabled);
    
    // 目标窗口
    std::wstring GetTargetWindowTitle() const;
    void SetTargetWindowTitle(const std::wstring& title);
    
    // 窗口位置
    int GetWindowX() const;
    int GetWindowY() const;
    void SetWindowPos(int x, int y);
    
    // 常用快捷键
    const std::set<int>& GetFavoriteHotkeys() const;
    void SetFavoriteHotkeys(const std::set<int>& favorites);
    void AddFavoriteHotkey(int index);
    void RemoveFavoriteHotkey(int index);
    bool IsFavoriteHotkey(int index) const;
    
private:
    Config();
    ~Config();
    
    std::set<std::wstring> m_enabledCategories;
    std::wstring m_targetWindowTitle;
    int m_windowX;
    int m_windowY;
    std::set<int> m_favoriteHotkeys;  // 常用快捷键索引集合
};
