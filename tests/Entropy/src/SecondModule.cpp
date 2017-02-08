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
    TestComponentHolder(u32 num) :
        mNum{num}
    {
        std::cout << "TestComponentHolder : " << mNum << " " << mInstantiated << std::endl;
        mInstantiated++;
    }
    u32 mNum;

    static u64 mInstantiated;

    ComponentT* add(ent::EntityId id) noexcept override
    { return nullptr; }
    ComponentT* get(ent::EntityId id) noexcept override
    { return nullptr; }
    bool has(ent::EntityId id) const noexcept override
    { return false; }
    void remove(ent::EntityId id) noexcept override
    { return; }
};

template <typename ComponentT,
    typename NUM>
u64 TestComponentHolder<ComponentT, NUM>::mInstantiated{0};

class TestSystem: public ent::System
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
        std::cout << "TestComponent1" << std::endl;
    }
    u32 x, y;
};

struct TestComponent4
{
    using HolderT = TestComponentHolder<TestComponent4, int>;

    TestComponent4()
    {
        std::cout << "TestComponent1" << std::endl;
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

        TC_Require(u.addSystem<TestSystem>(1).mNum == 1);
        TC_RequireNoException(u.removeSystem<TestSystem>());

        u64 id1{(u.registerComponent<TestComponent3>())};
        u64 id2{(u.registerComponent<TestComponent4>(1))};
        TC_RequireEqual(id1, 0u);
        TC_RequireEqual(id2, 1u);

        TC_RequireEqual((TestComponentHolder<TestComponent4, int>::mInstantiated), 1u);
    }
TU_End(EntropyEntityModule)

