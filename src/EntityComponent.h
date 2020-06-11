#pragma once

// base class for all components that can be attached to entity
class EntityComponent: public cxx::noncopyable
{
    decl_rtti_base(EntityComponent) // enable rtti for components

public:
    // readonly
    Entity* mParentEntity;

public:
    // try to cast base component type to derived
    template<typename TComponent>
    inline TComponent* CastComponent()
    {
        return cxx::rtti_cast<TComponent>(this);
    }

    // enable or disable component
    // @param isEnable: New enable state
    void EnableComponent(bool isEnable);
    // test whether component is enabled
    inline bool IsComponentActive() const { return mComponentEnabled; }

    // handle update frame
    // @param deltaTime: Time since last update
    virtual void UpdateComponent(float deltaTime) = 0;
    // component is attached to entity
    virtual void AwakeComponent() = 0;
    // destroy component instance
    virtual void DeleteComponent() = 0;

protected:
    // base component does not meant to be instantiated directly
    EntityComponent(Entity* parentEntity)
        : mParentEntity(parentEntity)
    {
        debug_assert(mParentEntity);
    }
    // use DeleteComponent to destroy it
    virtual ~EntityComponent()
    {
    }

    // overridables
    virtual void OnComponentEnabled();
    virtual void OnComponentDisabled();

protected:
    bool mComponentEnabled = true; // initially enabled
};