#pragma once

#include "ToolsUIWindow.h"

// forwards
struct ImGuiIO;

// immediate mode gui manager
class ToolsUIManager: public cxx::noncopyable
{
public:

    // setup ingui manager internal resources
    bool Initialize();
    void Deinit();

    bool IsInitialized() const;

    // render 
    void RenderFrame();

    // process single frame logic
    void UpdateFrame();

    // process input event
    // @param inputEvent: Event data
    void ProcessInputEvent(MouseButtonInputEvent& inputEvent);
    void ProcessInputEvent(MouseMovedInputEvent& inputEvent);
    void ProcessInputEvent(MouseScrollInputEvent& inputEvent);
    void ProcessInputEvent(KeyInputEvent& inputEvent);
    void ProcessInputEvent(KeyCharEvent& inputEvent);

    // add or remove debug window to global debug windows list
    // @param debugWindow: Window
    void AttachWindow(ToolsUIWindow* debugWindow);
    void DetachWindow(ToolsUIWindow* debugWindow);

    void HandleScreenResolutionChanged();

private:

    bool AddFontFromExternalFile(ImGuiIO& imguiIO, const char* fontFile, float fontSize);
    void SetupStyle(ImGuiIO& imguiIO);

private:
    // render data
    GpuBuffer* mVerticesBuffer = nullptr;
    GpuBuffer* mIndicesBuffer = nullptr;

    std::list<ToolsUIWindow*> mAllWindowsList;
};

extern ToolsUIManager gToolsUIManager;
