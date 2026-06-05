#include "nethack_hotkeys.h"
#include <map>
#include <windows.h>

// NetHack 快捷键数据
static std::vector<HotkeyItem> g_hotkeys = {
    // 移动
    {L"4", L"向左移动", L"movement", '4', false, false, false},
    {L"2", L"向下移动", L"movement", '2', false, false, false},
    {L"8", L"向上移动", L"movement", '8', false, false, false},
    {L"6", L"向右移动", L"movement", '6', false, false, false},
    {L"7", L"向左上移动", L"movement", '7', false, false, false},
    {L"9", L"向右上移动", L"movement", '9', false, false, false},
    {L"1", L"向左下移动", L"movement", '1', false, false, false},
    {L"3", L"向右下移动", L"movement", '3', false, false, false},
    {L".", L"等待一回合", L"movement", VK_OEM_PERIOD, false, false, false},
    {L"g", L"一直前进", L"movement", 'G', false, false, false},
    {L"<", L"向上走（楼梯）", L"movement", VK_OEM_COMMA, true, false, false},
    {L">", L"向下走（楼梯）", L"movement", VK_OEM_PERIOD, true, false, false},
    {L"Ctrl+T", L"传送", L"movement", 'T', false, true, false},
    {L"S", L" 搜索", L"movement", 'S', false, false, false},

    // 攻击
    {L"f", L"发射", L"attack", 'F', false, false, false},
    {L"F", L"强制攻击", L"attack", 'F', true, false, false},
    {L"t", L"投掷物品", L"attack", 'T', false, false, false},
    {L"Ctrl+D", L"踢", L"attack", 'D', false, true, false},
    {L"Q", L"填装弹药", L"attack", 'Q', true, false, false},
    {L"z", L"挥动魔杖", L"attack", 'Z', false, false, false},
    {L"Z", L"施展魔法", L"attack", 'Z', true, false, false},
    
    // 物品操作
    {L"i", L"查看背包", L"items", 'I', false, false, false},
    {L"I", L"查看背包（详细）", L"items", 'I', true, false, false},
    {L"\\", L"查看已发现", L"items", VK_OEM_5, false, false, false},
    {L"/", L"识别物品", L"items", VK_OEM_2, false, false, false},
    {L",", L"捡起物品", L"items", VK_OEM_COMMA, false, false, false},
    {L"a", L"使用物品", L"items", 'A', false, false, false},
    {L"A", L"脱下所有防具", L"items", 'A', true, false, false},
    {L"c", L"关门", L"items", 'C', false, false, false},
    {L"C", L"命名", L"items", 'C', true, false, false},
    {L"d", L"丢弃物品", L"items", 'D', false, false, false},
    {L"D", L"丢弃某种物品", L"items", 'D', true, false, false},
    {L"e", L"吃（食物）", L"items", 'E', false, false, false},
    {L"E", L"engrave 刻字", L"items", 'E', true, false, false},
    {L"o", L"开门", L"items", 'O', false, false, false},
    {L"p", L"付款", L"items", 'P', false, false, false},
    {L"P", L"佩戴饰品", L"items", 'P', true, false, false},
    {L"q", L"喝（药剂等）", L"items", 'Q', false, false, false},
    {L"r", L"阅读（卷轴）", L"items", 'R', false, false, false},
    {L"R", L"摘下饰品", L"items", 'R', true, false, false},
    {L"T", L"脱下装备", L"items", 'T', true, false, false},
    {L"w", L"装备武器", L"items", 'W', false, false, false},
    {L"W", L"穿戴防具", L"items", 'W', true, false, false},
    {L"x", L"交换武器", L"items", 'X', false, false, false},
    
    // 额外命令
    {L"Alt+a", L"调整背包排序", L"extra", 'A', false, false, true},
    {L"Alt+c", L"聊天", L"extra", 'C', false, false, true},
    {L"Alt+d", L"浸泡物品", L"extra", 'D', false, false, true},
    {L"Alt+e", L"加强", L"extra", 'E', false, false, true},
    {L"Alt+f", L"撬锁", L"extra", 'F', false, false, true},
    {L"Alt+i", L"唤醒", L"extra", 'I', false, false, true},
    {L"Alt+j", L"跳跃", L"extra", 'J', false, false, true},
    {L"Alt+l", L"搜刮", L"extra", 'L', false, false, true},
    {L"Alt+m", L"使用生物能力", L"extra", 'M', false, false, true},
    {L"Alt+n", L"命名", L"extra", 'N', false, false, true},
    {L"Alt+o", L"献祭", L"extra", 'O', false, false, true},
    {L"Alt+p", L"祈祷", L"extra", 'P', false, false, true},
    {L"Alt+q", L"退出", L"extra", 'Q', false, false, true},
    {L"Alt+r", L"擦拭", L"extra", 'R', false, false, true},
    {L"Alt+s", L"坐下", L"extra", 'S', false, false, true},
    {L"Alt+t", L"超度亡灵", L"extra", 'T', false, false, true},
    {L"Alt+u", L"解除陷阱", L"extra", 'U', false, false, true},
    {L"Alt+v", L"显示版本", L"extra", 'V', false, false, true},
    {L"Alt+w", L"擦擦脸", L"extra", 'W', false, false, true},




    // 状态/信息
    {L"Ctrl+X", L"查看角色属性", L"status", 'X', false, true, false},
    {L"[", L"查看当前防具", L"status", VK_OEM_4, false, false, false},
    {L"(", L"查看当前工具", L"status", '9', true, false, false},
    {L")", L"查看当前武器", L"status", '0', true, false, false},
    {L"=", L"查看当前魔法", L"status", VK_OEM_PLUS, false, false, false},
    {L"\"", L"查看当前护身符", L"status", VK_OEM_7, true, false, false},
    {L"*", L"查看装备清单", L"status", '8', true, false, false},
    {L"+", L"查看已学魔法", L"status", VK_OEM_PLUS, false, false, false},
    {L":", L"查看当前位置", L"status", VK_OEM_1, false, false, false},
    {L";", L"查看远方物品", L"status", VK_OEM_1, true, false, false},

    // 特殊动作
    {L"X", L"切换双持", L"special", 'X', true, false, false},
    {L"@", L"自动拾取开关", L"special", '2', true, false, false},
    {L"#", L"扩展命令", L"special", '3', true, false, false},
    {L"Ctrl+A", L"重复上一个命令", L"special", 'A', false, true, false},
    {L"u", L"解除陷阱", L"special", 'U', false, false, false},
    {L"h", L"帮助", L"special", 'H', false, false, false},
    {L"j", L"跳跃", L"special", 'J', false, false, false},
    {L"k", L"踢", L"special", 'K', false, false, false},
    {L"l", L"搜刮", L"special", 'L', false, false, false},
    {L"N", L"命名", L"special", 'N', true, false, false},

    // 地图/系统
    { L"S", L"保存游戏", L"system", 'S', true, false, false },
    { L"O", L"查看选项", L"system", 'O', true, false, false },
    { L"v", L"显示版本号", L"system", 'V', false, false, false },
    { L"V", L"显示游戏历史", L"system", 'V', true, false, false },
    { L"Ctrl+R", L"刷新屏幕", L"system", 'R', false, true, false },
    { L"Ctrl+P", L"重复上一条消息", L"system", 'P', false, true, false },
    { L"&", L"查看命令", L"system", '7', true, false, false },
    { L"^", L"找出陷阱", L"system", '6', false, false, false },
    { L"?", L"查看帮助", L"system", VK_OEM_2, true, false, false },
    { L"Ctrl+H", L"查看帮助", L"system", 'H', false, true, false },
    { L"Ctrl+L", L"重绘屏幕", L"system", 'L', false, true, false },
};

static std::vector<Category> g_categories = {
    {L"movement", L"移动", true},
    {L"attack", L"攻击", true},
    {L"items", L"物品操作", true},
    {L"extra", L"额外命令", true},
    {L"status", L"状态信息", true},
    {L"special", L"特殊动作", true},
    {L"system", L"系统", true},
};

const std::vector<HotkeyItem>& GetHotkeys() {
    return g_hotkeys;
}

const std::vector<Category>& GetCategories() {
    return g_categories;
}

std::vector<HotkeyItem> GetHotkeysByCategory(const std::wstring& categoryId) {
    std::vector<HotkeyItem> result;
    for (const auto& item : g_hotkeys) {
        if (item.category == categoryId) {
            result.push_back(item);
        }
    }
    return result;
}

std::wstring GetCategoryName(const std::wstring& categoryId) {
    for (const auto& cat : g_categories) {
        if (cat.id == categoryId) {
            return cat.name;
        }
    }
    return categoryId;
}
