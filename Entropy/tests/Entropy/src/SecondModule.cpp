/**
 * @file test/SecondModule.cpp
 * @author Tomas Polasek
 * @brief Testing functionality from second module.
 */

#include "SecondModule.h"

template <typename ComponentT,
    typename NUM>
class TestComponentHolder : public ent::BaseComponentHolder<ComponentT>
{
public:
    explicit TestComponentHolder(u32 num) :
        mNum{num}
    {
        mInstantiated++;
    }
    u32 mNum;

    static u64 mInstantiated;

    ComponentT* add(ent::EntityId id) noexcept override
    { return nullptr; }
    ComponentT* replace(ent::EntityId id, const ComponentT &comp) noexcept override
    { return nullptr; }
    ComponentT* get(ent::EntityId id) noexcept override
    { return nullptr; }
    const ComponentT* get(ent::EntityId id) const noexcept override
    { return nullptr; }
    bool remove(ent::EntityId id) noexcept override
    { return true; }
    void refresh() noexcept override
    { }
};

template <typename ComponentT,
    typename NUM>
u64 TestComponentHolder<ComponentT, NUM>::mInstantiated{0};

class TestSystem: public FirstUniverse::SystemT
{
public:
    TestSystem(u32 num) :
        mNum{num} {}
    u32 mNum;
};

struct TestComponent3
{
    TestComponent3()
    {
    }
    u32 x, y;
};

struct TestComponent4
{
    using HolderT = TestComponentHolder<TestComponent4, double>;

    TestComponent4()
    {
    }
    u32 x, y;
};

TU_Begin(EntropyEntityModule)

    TU_Setup
    {

    }

    TU_Teardown
    {

    }

    TU_Case(UniverseSecondModule, "Testing the Universe class in second module")
    {
        PROF_SCOPE("UniverseSecondModule");

        using Universe = FirstUniverse;
        Universe::UniverseT &u{Universe::instance()};

        u64 id1{(u.registerComponent<TestComponent3>())};
        u64 id2{(u.registerComponent<TestComponent4>(1))};
        TC_RequireEqual(id1, 0u);
        TC_RequireEqual(id2, 1u);

        u.init();

        TC_Require(u.addSystem<TestSystem>(1)->mNum == 1);
        TC_RequireNoException(u.removeSystem<TestSystem>());

        TC_RequireEqual((TestComponentHolder<TestComponent4, double>::mInstantiated), 1u);
    }
TU_End(EntropyEntityModule)

