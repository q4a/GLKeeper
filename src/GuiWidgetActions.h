#pragma once

#include "GuiDefs.h"

// forwards
class GuiWidgetAction;
class GuiWidgetActionsHolder;

//////////////////////////////////////////////////////////////////////////

// base widget action class
class GuiWidgetAction: public cxx::noncopyable
{
public:
    void PerformAction(GuiWidget* parentWidget);
    void ReleaseAction();
    bool Deserialize(cxx::json_node_object actionNode);
    GuiWidgetAction* CloneAction();

protected:
    GuiWidgetAction() = default;
    GuiWidgetAction(const GuiWidgetAction* copyAction);
    virtual ~GuiWidgetAction()
    {
    }

    bool EvaluateConditions(GuiWidget* parentWidget) const;

protected:
    // overridables
    virtual void HandlePerformAction(GuiWidget* targetWidget) = 0;
    virtual bool HandleDeserialize(cxx::json_node_object actionNode)
    {
        return true;
    }
    virtual GuiWidgetAction* HandleCloneAction() = 0;

protected:
    // common properties
    cxx::logical_expression mConditions;
    std::string mTargetPath;
};

//////////////////////////////////////////////////////////////////////////

// widget actions holder
class GuiWidgetActionsHolder: public cxx::noncopyable
{
public:
    // readonly
    GuiWidget* mParentWidget;

public:
    GuiWidgetActionsHolder(GuiWidget* actionsParentWidget);
    ~GuiWidgetActionsHolder();

    // add action to controller
    void AddAction(cxx::unique_string eventId, GuiWidgetAction* action);
    void ClearActions();
    void CopyActions(const GuiWidgetActionsHolder& source);

    // invoke actions associated with event id
    void EmitEvent(cxx::unique_string eventId);

private:
    struct EventActionStruct
    {
    public:
        cxx::unique_string mEventId;
        GuiWidgetAction* mAction; // action associated with event
    };
    std::vector<EventActionStruct> mActionsList;
};

//////////////////////////////////////////////////////////////////////////

// actions manager
class GuiWidgetActionsManager: public cxx::noncopyable
{
public:
    // try load single widget action from json document node
    // @returns null on error
    GuiWidgetAction* DeserializeAction(cxx::json_node_object actionNode);
};

extern GuiWidgetActionsManager gGuiWidgetActionsManager;