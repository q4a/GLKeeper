#include "pch.h"
#include "ImGuiManager.h"
#include "imgui.h"
#include "GraphicsDevice.h"
#include "FileSystem.h"
#include "BinaryInputStream.h"
#include "InputsManager.h"
#include "TimeManager.h"
#include "GpuBuffer.h"

ImGuiManager gImGuiManager;

// imgui specific data size constants
const unsigned int Sizeof_ImGuiVertex = sizeof(ImDrawVert);
const unsigned int Sizeof_ImGuiIndex = sizeof(ImDrawIdx);

bool ImGuiManager::Initialize()
{
    if (mInitialized)
        return true;

    // initialize imgui context
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr; // disable saving state
    io.LogFilename = nullptr; // disable saving log

    io.KeyMap[ImGuiKey_Tab]         = eKeycode_TAB;
    io.KeyMap[ImGuiKey_LeftArrow]   = eKeycode_LEFT;
    io.KeyMap[ImGuiKey_RightArrow]  = eKeycode_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow]     = eKeycode_UP;
    io.KeyMap[ImGuiKey_DownArrow]   = eKeycode_DOWN;
    io.KeyMap[ImGuiKey_PageUp]      = eKeycode_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown]    = eKeycode_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home]        = eKeycode_HOME;
    io.KeyMap[ImGuiKey_End]         = eKeycode_END;
    io.KeyMap[ImGuiKey_Insert]      = eKeycode_INSERT;
    io.KeyMap[ImGuiKey_Delete]      = eKeycode_DELETE;
    io.KeyMap[ImGuiKey_Backspace]   = eKeycode_BACKSPACE;
    io.KeyMap[ImGuiKey_Space]       = eKeycode_SPACE;
    io.KeyMap[ImGuiKey_Enter]       = eKeycode_ENTER;
    io.KeyMap[ImGuiKey_Escape]      = eKeycode_ESCAPE;

    io.BackendRendererName = "imgui_impl_impsengine";
    io.BackendFlags = ImGuiBackendFlags_HasSetMousePos;
    io.ConfigFlags = ImGuiConfigFlags_NavEnableSetMousePos;

    SetupStyle(io);

    Size2D fontTextureDimensions;
    unsigned char *pcPixels;

    if (!AddFontFromExternalFile(io, "fonts/cousine_regular.ttf", 25.0f))
    {
        Deinit();
        return false;
    }

    io.Fonts->Build();
    io.Fonts->GetTexDataAsRGBA32(&pcPixels, &fontTextureDimensions.x, &fontTextureDimensions.y);

    GpuTexture2D* fontTexture = gGraphicsDevice.CreateTexture2D(eTextureFormat_RGBA8, fontTextureDimensions, pcPixels);
    debug_assert(fontTexture);

    mVertsBuffer = gGraphicsDevice.CreateBuffer(eBufferContent_Vertices);
    debug_assert(mVertsBuffer);

    mTrisBuffer = gGraphicsDevice.CreateBuffer(eBufferContent_Indices);
    debug_assert(mTrisBuffer);

    io.Fonts->TexID = fontTexture;

    mInitialized = true;
    return mInitialized;
}

void ImGuiManager::Deinit()
{
    if (mInitialized)
    {
        ImGuiIO& io = ImGui::GetIO();

        // destroy font texture
        GpuTexture2D* fontTexture = static_cast<GpuTexture2D*>(io.Fonts->TexID);
        if (fontTexture)
        {
            gGraphicsDevice.DestroyTexture(fontTexture);
            io.Fonts->TexID = nullptr;
        }

        ImGui::DestroyContext();
        mInitialized = false;
    }

    if (mTrisBuffer)
    {
        gGraphicsDevice.DestroyBuffer(mTrisBuffer);
        mTrisBuffer = nullptr;
    }

    if (mVertsBuffer)
    {
        gGraphicsDevice.DestroyBuffer(mVertsBuffer);
        mVertsBuffer = nullptr;
    }
}

void ImGuiManager::RenderFrame()
{
    if (!mInitialized)
        return;

    ImDrawData* imGuiDrawData = ImGui::GetDrawData();

    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    int fb_width = (int)(imGuiDrawData->DisplaySize.x * imGuiDrawData->FramebufferScale.x);
    int fb_height = (int)(imGuiDrawData->DisplaySize.y * imGuiDrawData->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0)
        return;

    ImVec2 clip_off = imGuiDrawData->DisplayPos;         // (0,0) unless using multi-viewports
    ImVec2 clip_scale = imGuiDrawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

                                                         // imgui primitives rendering
    for (int iCommandList = 0; iCommandList < imGuiDrawData->CmdListsCount; ++iCommandList)
    {
        const ImDrawList* cmd_list = imGuiDrawData->CmdLists[iCommandList];
        Vertex2D_Format vertexFormat;
        // vertex / index buffer generated by Dear ImGui

        if (!mVertsBuffer->Setup(eBufferUsage_Stream, cmd_list->VtxBuffer.size_in_bytes(), cmd_list->VtxBuffer.Data))
        {
            debug_assert(false);
        }

        if (!mTrisBuffer->Setup(eBufferUsage_Stream, cmd_list->IdxBuffer.size_in_bytes(), cmd_list->IdxBuffer.Data))
        {
            debug_assert(false);
        }
        gGraphicsDevice.BindVertexBuffer(mVertsBuffer, vertexFormat);
        gGraphicsDevice.BindIndexBuffer(mTrisBuffer);

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; ++cmd_i)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
                continue;
            }

            // Project scissor/clipping rectangles into framebuffer space
            float clip_rect_x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
            float clip_rect_y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
            float clip_rect_w = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
            float clip_rect_h = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

            bool should_draw = (clip_rect_x < fb_width && clip_rect_y < fb_height && clip_rect_w >= 0.0f && clip_rect_h >= 0.0f);
            if (!should_draw)
                continue;

            Rect2D rcClip {
                static_cast<int>(clip_rect_x), static_cast<int>(fb_height - clip_rect_h), 
                static_cast<int>(clip_rect_w - clip_rect_x), 
                static_cast<int>(clip_rect_h - clip_rect_y)
            };

            GpuTexture2D* bindTexture = static_cast<GpuTexture2D*>(pcmd->TextureId);
            gGraphicsDevice.BindTexture(eTextureUnit_0, bindTexture);

            gGraphicsDevice.SetScissorRect(rcClip);
            unsigned int idxBufferOffset = Sizeof_ImGuiIndex * pcmd->IdxOffset;

            eIndicesType indicesType = Sizeof_ImGuiIndex == 2 ? eIndicesType_i16 : eIndicesType_i32;
            gGraphicsDevice.RenderIndexedPrimitives(ePrimitiveType_Triangles, indicesType, idxBufferOffset, pcmd->ElemCount);
        }
    }
}

void ImGuiManager::UpdateFrame()
{
    if (!mInitialized)
        return;

    ImGuiIO& io = ImGui::GetIO();

    io.DeltaTime = (float) gTimeManager.GetRealtimeFrameDelta(); // set the time elapsed since the previous frame (in seconds)
    io.DisplaySize.x = (float) gGraphicsDevice.mViewportRect.mSizeX;
    io.DisplaySize.y = (float) gGraphicsDevice.mViewportRect.mSizeY;
    io.MousePos.x = gInputsManager.mCursorPositionX * 1.0f;
    io.MousePos.y = gInputsManager.mCursorPositionY * 1.0f;
    io.MouseDown[0] = gInputsManager.GetMouseButtonL();  // set the mouse button states
    io.MouseDown[1] = gInputsManager.GetMouseButtonR();
    io.MouseDown[3] = gInputsManager.GetMouseButtonM();

    ImGui::NewFrame();

    if (true)
    {
        ImGui::ShowDemoWindow();
    }

    //for (BLImGuiWindow* currWindow: mWindowsList)
    //{
    //    currWindow->DoUI(*this, dt);
    //}

    ImGui::Render();
}

void ImGuiManager::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
    if (!mInitialized)
        return;

    ImGuiIO& io = ImGui::GetIO();

    if (io.WantCaptureMouse)
    {
        inputEvent.SetConsumed();
    }
}

void ImGuiManager::HandleInputEvent(MouseMovedInputEvent& inputEvent)
{
    if (!mInitialized)
        return;

    ImGuiIO& io = ImGui::GetIO();

    if (io.WantCaptureMouse)
    {
        inputEvent.SetConsumed();
    }
}

void ImGuiManager::HandleInputEvent(MouseScrollInputEvent& inputEvent)
{
    if (!mInitialized)
        return;

    ImGuiIO& io = ImGui::GetIO();

    io.MouseWheelH += inputEvent.mScrollX * 1.0f;
    io.MouseWheel += inputEvent.mScrollY * 1.0f;

    if (io.WantCaptureMouse)
    {
        inputEvent.SetConsumed();
    }
}

void ImGuiManager::HandleInputEvent(KeyInputEvent& inputEvent)
{
    if (!mInitialized)
        return;

    ImGuiIO& io = ImGui::GetIO();

    io.KeysDown[inputEvent.mKeycode] = inputEvent.mPressed;
    io.KeyCtrl = inputEvent.HasModifiers(KeyModifier_Ctrl);
    io.KeyShift = inputEvent.HasModifiers(KeyModifier_Shift);
    io.KeyAlt = inputEvent.HasModifiers(KeyModifier_Alt);

    if (io.WantCaptureKeyboard)
    {
        inputEvent.SetConsumed();
    }
}

void ImGuiManager::HandleInputEvent(KeyCharEvent& inputEvent)
{
    if (!mInitialized)
        return;

    ImGuiIO& io = ImGui::GetIO();

    io.AddInputCharacter(inputEvent.mUnicodeChar);

    if (io.WantTextInput)
    {
        inputEvent.SetConsumed();
    }
}

bool ImGuiManager::IsInitialized() const
{
    return mInitialized;
}

bool ImGuiManager::AddFontFromExternalFile(ImGuiIO& imguiIO, const char* fontFile, float fontSize)
{
    bool isSuccess = false;

    if (BinaryInputStream* fileStream = gFileSystem.OpenFileStream(fontFile))
    {
        long fileStreamLength = fileStream->GetLength();
        if (fileStreamLength > 0)
        {
            ByteArray dataBuffer;
            dataBuffer.resize(fileStreamLength);

            long readBytes = fileStream->ReadData(dataBuffer.data(), fileStreamLength);
            debug_assert(readBytes == fileStreamLength);

            ImFont* imFont = imguiIO.Fonts->AddFontFromMemoryTTF(dataBuffer.data(), fileStreamLength, fontSize);
            if (imFont)
            {
                isSuccess = true;
            }
            debug_assert(imFont);
        }

        gFileSystem.CloseFileStream(fileStream);
    }
    return isSuccess;
}

void ImGuiManager::SetupStyle(ImGuiIO& imguiIO)
{
    //ImGui::StyleColorsDark();
    ImGuiStyle & style = ImGui::GetStyle();
    ImVec4 * colors = style.Colors;

    /// 0 = FLAT APPEARENCE
    /// 1 = MORE "3D" LOOK
    int is3D = 1;

    colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_Border]                 = ImVec4(0.12f, 0.12f, 0.12f, 0.71f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.42f, 0.42f, 0.42f, 0.54f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.42f, 0.42f, 0.42f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.56f, 0.56f, 0.56f, 0.67f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.17f, 0.17f, 0.17f, 0.90f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.335f, 0.335f, 0.335f, 1.000f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.24f, 0.24f, 0.24f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.64f, 0.64f, 0.64f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.54f, 0.54f, 0.54f, 0.35f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.52f, 0.52f, 0.52f, 0.59f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.76f, 0.76f, 0.76f, 0.77f);
    colors[ImGuiCol_Separator]              = ImVec4(0.000f, 0.000f, 0.000f, 0.137f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.700f, 0.671f, 0.600f, 0.290f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.702f, 0.671f, 0.600f, 0.674f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.73f, 0.73f, 0.73f, 0.35f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);

    style.PopupRounding     = 3;
    style.WindowPadding     = ImVec2(4, 4);
    style.FramePadding      = ImVec2(6, 4);
    style.ItemSpacing       = ImVec2(6, 2);
    style.ScrollbarSize     = 15;
    style.WindowBorderSize  = 1;
    style.ChildBorderSize   = 1;
    style.PopupBorderSize   = 1;
    style.IndentSpacing     = 22.0f;
    style.FrameBorderSize   = is3D * 1.0f; 
    style.WindowRounding    = 2;
    style.ChildRounding     = 2;
    style.FrameRounding     = 2;
    style.ScrollbarRounding = 2;
    style.GrabRounding      = 2;
    style.TabBorderSize     = is3D * 1.0f; 
    style.TabRounding       = 2;

#ifdef IMGUI_HAS_DOCK 
    colors[ImGuiCol_DockingEmptyBg]     = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
    colors[ImGuiCol_Tab]                = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_TabHovered]         = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_TabActive]          = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
    colors[ImGuiCol_TabUnfocused]       = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
    colors[ImGuiCol_DockingPreview]     = ImVec4(0.85f, 0.85f, 0.85f, 0.28f);

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
#endif
}