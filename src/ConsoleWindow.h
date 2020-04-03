#pragma once

#include "DebugUIWindow.h"

// forwards
struct ImGuiInputTextCallbackData;

// debug console window
class ConsoleWindow: public DebugUIWindow
{
public:
    bool mAutoScroll = true;
    bool mScrollToBottom = false;

public:
    ConsoleWindow();

private:
    // override DebugGuiWindow
    void DoUI(ImGuiIO& imguiContext) override;

    // internals
    int TextEditCallback(ImGuiInputTextCallbackData* data);
    void Exec();
    void MoveInputToHistory();

private:
    std::string mInputString;
    std::deque<std::string> mHistory;
    int mHistoryPos = -1; // -1: new line, 0..History.Size-1 browsing history.
};

extern ConsoleWindow gConsoleWindow;