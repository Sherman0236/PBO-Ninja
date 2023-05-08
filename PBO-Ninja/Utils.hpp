#pragma once
#include <windows.h>
#include <filesystem>

namespace Utils
{
    /// <summary>
    /// Small wrapper for MessageBoxA
    /// </summary>
    inline int ShowMessageBox(std::string title, std::string text, int type = MB_ICONINFORMATION)
    {
        return MessageBoxA(nullptr, text.c_str(), title.c_str(), type);
    }
}