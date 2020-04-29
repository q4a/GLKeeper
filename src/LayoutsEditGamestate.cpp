#include "pch.h"
#include "LayoutsEditGamestate.h"
#include "GameMain.h"
#include "GuiManager.h"
#include "GuiButton.h"
#include "GuiPictureBox.h"
#include "GraphicsDevice.h"
#include "GuiPanel.h"
#include "GuiScrollbar.h"
#include "TexturesManager.h"
#include "ToolsUIManager.h"
#include "Console.h"

LayoutsEditGamestate::LayoutsEditGamestate()
{

}

void LayoutsEditGamestate::HandleGamestateEnter()
{
    gToolsUIManager.AttachWindow(&mLayoutsEditWindow);
    gGuiManager.AttachWidgets(&mHier);

    if (!mHier.LoadFromFile("layouts/test.json"))
    {
        debug_assert(false);
    }

    if (GuiWidget* buttonWidget = mHier.SearchForWidget("button_0"))
    {
        Subscribe(GuiEventId_Click, buttonWidget);
        Subscribe(GuiEventId_MouseDown, buttonWidget);
        Subscribe(GuiEventId_MouseUp, buttonWidget);
        Subscribe(GuiEventId_MouseEnter, buttonWidget);
        Subscribe(GuiEventId_MouseLeave, buttonWidget);
        Subscribe(cxx::unique_string("custom_on_click_event"), buttonWidget);

        SetupVisibility(buttonWidget);
    }

    if (GuiWidget* buttonWidget = mHier.SearchForWidget("button_1"))
    {
        Subscribe(GuiEventId_Click, buttonWidget);
        Subscribe(GuiEventId_MouseDown, buttonWidget);
        Subscribe(GuiEventId_MouseUp, buttonWidget);
        Subscribe(GuiEventId_MouseEnter, buttonWidget);
        Subscribe(GuiEventId_MouseLeave, buttonWidget);
        Subscribe(cxx::unique_string("custom_on_click_event"), buttonWidget);

        SetupVisibility(buttonWidget);
    }

    GuiWidget* sliderThumb = mHier.GetWidgetByPath("root/slider_0/#slider");
    Subscribe(GuiEventId_MouseDown, sliderThumb);
}

void LayoutsEditGamestate::HandleGamestateLeave()
{
    gToolsUIManager.DetachWindow(&mLayoutsEditWindow);

    UnsubscribeAll();

    gGuiManager.DetachWidgets(&mHier);
    mHier.Cleanup();
}

void LayoutsEditGamestate::HandleUpdateFrame()
{
}

void LayoutsEditGamestate::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
}

void LayoutsEditGamestate::HandleInputEvent(MouseMovedInputEvent& inputEvent)
{
}

void LayoutsEditGamestate::HandleInputEvent(MouseScrollInputEvent& inputEvent)
{
}

void LayoutsEditGamestate::HandleInputEvent(KeyInputEvent& inputEvent)
{
    if (inputEvent.HasPressed(eKeycode_F5))
    {
        if (!mHier.LoadFromFile("layouts/test.json"))
        {
            debug_assert(false);
        }
    }
}

void LayoutsEditGamestate::HandleInputEvent(KeyCharEvent& inputEvent)
{
}

void LayoutsEditGamestate::HandleClick(GuiWidget* sender)
{
    debug_assert(sender);
    gConsole.LogMessage(eLogMessage_Debug, "on_click %s", sender->mName.c_str());
}

void LayoutsEditGamestate::HandleMouseEnter(GuiWidget* sender)
{
    debug_assert(sender);
    gConsole.LogMessage(eLogMessage_Debug, "on_mouse_enter %s", sender->mName.c_str());
    SetupVisibility(sender);
}

void LayoutsEditGamestate::HandleMouseLeave(GuiWidget* sender)
{
    debug_assert(sender);
    gConsole.LogMessage(eLogMessage_Debug, "on_mouse_leave %s", sender->mName.c_str());
    SetupVisibility(sender);
}

void LayoutsEditGamestate::HandleMouseDown(GuiWidget* sender, eMouseButton mbutton)
{
    debug_assert(sender);
    gConsole.LogMessage(eLogMessage_Debug, "on_mouse_down %s", sender->mName.c_str());
    SetupVisibility(sender);
}

void LayoutsEditGamestate::HandleMouseUp(GuiWidget* sender, eMouseButton mbutton)
{
    debug_assert(sender);
    gConsole.LogMessage(eLogMessage_Debug, "on_mouse_up %s", sender->mName.c_str());
    SetupVisibility(sender);
}

void LayoutsEditGamestate::HandleEvent(GuiWidget* sender, cxx::unique_string eventId)
{
    debug_assert(sender);
    gConsole.LogMessage(eLogMessage_Debug, "on_event '%s'", eventId.c_str());
}

void LayoutsEditGamestate::SetupVisibility(GuiWidget* widget)
{
    GuiRefreshVisibilityRecursive(widget, [widget](const std::string& name)->bool
        {
            if (name == "hovered_state")
            {
                return widget->IsHovered();
            }
            if (name == "pressed_state")
            {
                return widget->IsSelected();
            }
            debug_assert(false);
            return false;
        });
}
