/**
 * @file test/Tests.cpp
 * @author Tomas Polasek
 * @brief Main testing file.
 */

#include "Tests.h"

template <typename ComponentT,
    typename NUM>
class TestComponentHolder : public ent::BaseComponentHolder<ComponentT>
{
public:
    TestComponentHolder(u32 num)
    {
        mNum = num;
        std::cout << "TestComponentHolder : " << mNum << " " << mInstantiated << std::endl;
        mInstantiated++;
    }
    static u32 mNum;
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
template <typename ComponentT,
    typename NUM>
u32 TestComponentHolder<ComponentT, NUM>::mNum{0};

class TestSystem: public ent::System
{
public:
    TestSystem(u32 num) :
        mNum{num} {}

    const auto &getFilter() const
    { return filter(); }

    auto getGroupId() const
    { return groupId(); }

    u32 mNum;
};

struct TestComponent1
{
    using HolderT = TestComponentHolder<TestComponent1, int>;

    TestComponent1()
    {
        std::cout << "TestComponent1" << std::endl;
    }
    u32 x, y;
};

struct TestComponent2
{
    using HolderT = TestComponentHolder<TestComponent2, int>;

    TestComponent2()
    {
        std::cout << "TestComponent1" << std::endl;
    }
    u32 x, y;
};

template <u64 N>
struct TestComponent
{
    u32 v;
};

template <u64 N>
class TestSystem2: public ent::System
{
public:
    using Require = ent::Require<TestComponent<0>>;
    using Reject = ent::Reject<TestComponent<1>>;

    TestSystem2(u32 num) :
        mNum{num} {}

    const auto &getFilter() const
    { return filter(); }

    auto getGroupId() const
    { return groupId(); }

    u32 mNum;
};

TU_Begin(EntropyEntity)

    TU_Setup
    {

    }

    TU_Teardown
    {

    }

    TU_Case(ClassIdGenerator0, "Testing the ClassIdGenerator class")
    {
        PROF_SCOPE("IdGenerator0");
        class GenA : public ent::ClassIdGenerator<GenA> {};
        TC_RequireConstexpr(!GenA::generated<u32>());
        TC_RequireConstexprEqual(GenA::getId<u32>(), 0);
        TC_RequireConstexpr(GenA::generated<u32>());
        TC_RequireConstexpr(!GenA::generated<u64>());
        TC_RequireConstexpr(!GenA::generated<double>());
        TC_RequireConstexpr(!GenA::generated<float>());
        TC_RequireConstexprEqual(GenA::getId<u64>(), 1);
        TC_RequireConstexprEqual(GenA::getId<double>(), 2);
        TC_RequireConstexprEqual(GenA::getId<float>(), 3);
        TC_RequireConstexprEqual(GenA::getId<u32>(), 0);
        TC_RequireConstexprEqual(GenA::getId<u64>(), 1);
        TC_RequireConstexpr(GenA::generated<u32>());
        TC_RequireConstexpr(GenA::generated<u64>());
        TC_RequireConstexpr(GenA::generated<double>());
        TC_RequireConstexpr(GenA::generated<float>());

        class GenB : public ent::ClassIdGenerator<GenB> {};
        TC_RequireConstexpr(!GenB::generated<double>());
        TC_RequireConstexprEqual(GenB::getId<double>(), 0);
        TC_RequireConstexpr(GenB::generated<double>());
        TC_RequireConstexpr(!GenB::generated<u32>());
        TC_RequireConstexpr(!GenB::generated<u64>());
        TC_RequireConstexpr(!GenB::generated<float>());
        TC_RequireConstexprEqual(GenB::getId<float>(), 1);
        TC_RequireConstexprEqual(GenB::getId<u32>(), 2);
        TC_RequireConstexprEqual(GenB::getId<u64>(), 3);
        TC_RequireConstexprEqual(GenB::getId<double>(), 0);
        TC_RequireConstexprEqual(GenB::getId<float>(), 1);
        TC_RequireConstexpr(GenB::generated<u32>());
        TC_RequireConstexpr(GenB::generated<u64>());
        TC_RequireConstexpr(GenB::generated<double>());
        TC_RequireConstexpr(GenB::generated<float>());

        class GenC : public ent::ClassIdGenerator<GenC, 5> {};
        TC_RequireConstexpr(!GenC::generated<double>());
        TC_RequireConstexprEqual(GenC::getId<double>(), 5);
        TC_RequireConstexpr(GenC::generated<double>());
        TC_RequireConstexpr(!GenC::generated<u32>());
        TC_RequireConstexpr(!GenC::generated<u64>());
        TC_RequireConstexpr(!GenC::generated<float>());
        TC_RequireConstexprEqual(GenC::getId<float>(), 6);
        TC_RequireConstexprEqual(GenC::getId<u32>(), 7);
        TC_RequireConstexprEqual(GenC::getId<u64>(), 8);
        TC_RequireConstexprEqual(GenC::getId<double>(), 5);
        TC_RequireConstexprEqual(GenC::getId<float>(), 6);
        TC_RequireConstexpr(GenC::generated<u32>());
        TC_RequireConstexpr(GenC::generated<u64>());
        TC_RequireConstexpr(GenC::generated<double>());
        TC_RequireConstexpr(GenC::generated<float>());
    }

    TU_Case(Universe0, "Testing the Universe class")
    {
        PROF_SCOPE("Universe0");
        using Universe = FirstUniverse;
        Universe::UniverseT &u{Universe::instance()};
        TC_Require(u.addSystem<TestSystem>(1)->mNum == 1);
        TC_RequireNoException(u.removeSystem<TestSystem>());
        u64 id1{(u.registerComponent<TestComponent1>(1))};
        u64 id2{(u.registerComponent<TestComponent2>(2))};
        TC_RequireEqual(id1, 2u);
        TC_RequireEqual(id2, 3u);
        TC_RequireEqual((TestComponentHolder<TestComponent1, int>::mInstantiated), 1u);
        TC_RequireEqual((TestComponentHolder<TestComponent1, int>::mNum), 1u);
        TC_RequireEqual((TestComponentHolder<TestComponent2, int>::mInstantiated), 1u);
        TC_RequireEqual((TestComponentHolder<TestComponent2, int>::mNum), 2u);
    }

    TU_Case(ComponentBitset0, "Testing the ComponentBitset class")
    {
        PROF_SCOPE("ComponentBitset0");
        ent::ComponentBitset cbs;
        TC_Require(!cbs.any());
        TC_Require(cbs.none());
        TC_Require(cbs.size() == ent::MAX_COMPONENTS);
        cbs.set(0);
        TC_Require(cbs.test(0) == true && cbs.any());
        TC_Require(cbs.all() == false);
        TC_Require(cbs.none() == false);
        cbs.reset();
        TC_Require(cbs.none() == true);
        ent::ComponentBitset cbs2(15);
        TC_Require(cbs2.count() == 4);
    }

    TU_Case(ComponentFilter0, "Testing the ComponentFilter class")
    {
        PROF_SCOPE("ComponentFilter0");
        ent::ComponentFilter filter(0b1010, 0b1100);
        TC_Require(filter.match(0b1011));
        TC_Require(filter.match(0b1010));
        TC_Require(!filter.match(0b1111));
        TC_Require(!filter.match(0b1001));
        TC_Require(!filter.match(0b0000));
        TC_Require(!filter.match(0b1100));
    }

    TU_Case(EntityId0, "Testing the EntityId class")
    {
        PROF_SCOPE("EntityId0");
        ent::EntityId eid;
        TC_Check(eid.id() == 0);
        TC_Check(eid.index() == 0);
        TC_Check(eid.generation() == 0);
    }

    TU_Case(EntityId1, "Testing the EntityId class")
    {
        PROF_SCOPE("EntityId1");
        TC_Require(ent::EID_GEN_BITS == 8 && ent::EID_INDEX_BITS == 24);
        ent::EIdType gen{123};
        ent::EIdType index{123};
        ent::EntityId eid(index, gen);
        TC_CheckEqual(eid.id(), ((gen << ent::EID_INDEX_BITS) | (index)));
        TC_CheckEqual(eid.index(), index);
        TC_CheckEqual(eid.generation(), gen);
    }

    TU_Case(EntityId2, "Testing the EntityId class")
    {
        PROF_SCOPE("EntityId2");
        TC_Require(ent::EID_GEN_BITS == 8 && ent::EID_INDEX_BITS == 24);
        ent::EIdType gen{(ent::EIdType(1) << ent::EID_GEN_BITS) - 1};
        ent::EIdType index{(ent::EIdType(1) << ent::EID_INDEX_BITS) - 1};
        ent::EntityId eid(index, gen);
        TC_CheckEqual(eid.id(), ((gen << ent::EID_INDEX_BITS) | (index)));
        TC_CheckEqual(eid.index(), index);
        TC_CheckEqual(eid.generation(), gen);
    }

    TU_Case(Entity0, "Testing the Entity class")
    {
        PROF_SCOPE("Entity0");
        using EntityT1 = FirstUniverse::EntityT;
        using EntityT2 = SecondUniverse::EntityT;
        FirstUniverse::UniverseT *un1{reinterpret_cast<FirstUniverse::UniverseT*>(1u)};
        SecondUniverse::UniverseT *un2{reinterpret_cast<SecondUniverse::UniverseT*>(2u)};
        EntityT1 ent11(un1, 1);
        EntityT2 ent21(un2, 2);
        TC_Require(ent11.universe() == un1 && ent11.id() == 1);
        TC_Require(ent21.universe() == un2 && ent21.id() == 2);
        EntityT1 ent12(ent11);
        EntityT2 ent22(ent21);
        TC_Require(ent12.universe() == un1 && ent12.id() == 1);
        TC_Require(ent22.universe() == un2 && ent22.id() == 2);
        TC_Require(ent12 == ent11);
        TC_Require(ent22 == ent21);
        ent12 = ent21;
        ent22 = ent11;
        TC_Require(ent12.universe() == un1 && ent12.id() == 2);
        TC_Require(ent22.universe() == un2 && ent22.id() == 1);
    }

    TU_Case(EntityHolder0, "Testing the EntityHolder class")
    {
        PROF_SCOPE("EntityHolder0");
        using ent::EntityHolder;
        using ent::EntityId;
        static constexpr u64 CREATE_NUM{100};
        EntityHolder h1;

        for (u64 iii = 1; iii <= CREATE_NUM; ++iii)
        {
            TC_RequireEqual(h1.create(), EntityId(iii, 0));
        }
        TC_Require(!h1.destroy(EntityId()));
        for (u64 iii = 1; iii <= ent::ENT_MIN_FREE; ++iii)
        {
            TC_RequireEqual(h1.create(), EntityId(CREATE_NUM + iii, 0));
            TC_Require(h1.destroy(EntityId(iii, 0)));
        }
        TC_RequireEqual(h1.create(), EntityId(1, 1));
        TC_RequireEqual(h1.create(), EntityId(CREATE_NUM + ent::ENT_MIN_FREE + 1, 0));
        TC_Require(h1.valid(EntityId(1, 1)));
        for (u64 iii = 2; iii <= ent::ENT_MIN_FREE; ++iii)
        {
            TC_Require(!h1.valid(EntityId(iii, 0)));
        }
        for (u64 iii = ent::ENT_MIN_FREE + 1; iii <= CREATE_NUM + ent::ENT_MIN_FREE + 1; ++iii)
        {
            TC_Require(h1.valid(EntityId(iii, 0)));
            TC_Require(h1.active(EntityId(iii, 0)));
        }
        TC_Require(!h1.valid(EntityId(CREATE_NUM + ent::ENT_MIN_FREE + 2)));

        EntityHolder h2;
        for (u64 iii = 1; iii <= 10; ++iii)
        {
            TC_RequireEqual(h2.create(), EntityId(iii, 0));
        }
        for (u64 iii = 1; iii <= 10; ++iii)
        {
            TC_Require(h2.valid(EntityId(iii, 0)));
            TC_Require(h2.active(EntityId(iii, 0)));
            h2.deactivate(EntityId(iii, 0));
            TC_Require(h2.valid(EntityId(iii, 0)));
            TC_Require(!h2.active(EntityId(iii, 0)));
            h2.activate(EntityId(iii, 0));
            TC_Require(h2.valid(EntityId(iii, 0)));
            TC_Require(h2.active(EntityId(iii, 0)));
        }
    }

    TU_Case(EntityManager0, "Testing the EntityManager class")
    {
        PROF_SCOPE("EntityManager0");
        FirstUniverse::UniverseT  &u{FirstUniverse::instance()};
        using Entity = FirstUniverse::EntityT;

        for (u64 iii = 1u; iii <= ent::ENT_MIN_FREE + 1; ++iii)
        {
            Entity ent = u.createEntity();
            TC_Require(ent.created());
            TC_Require(ent.active());
            TC_Require(ent.valid());
            if (iii == ent::ENT_MIN_FREE + 1)
            { TC_RequireEqual(ent.id(), ent::EntityId(1u, 1u)); }
            else
            { TC_RequireEqual(ent.id(), ent::EntityId(iii, 0u)); }

            ent.deactivate();
            TC_Require(!ent.active());
            TC_Require(ent.valid());

            ent.activate();
            TC_Require(ent.active());
            TC_Require(ent.valid());

            TC_Require(ent.destroy());
            TC_Require(!ent.valid());
            TC_Require(!ent.created());
        }
    }

    TU_Case(ComponentManager0, "Testing the ComponentHolder class")
    {
        PROF_SCOPE("ComponentManager0");
        SecondUniverse::UniverseT &u{SecondUniverse::instance()};
        TC_RequireEqual(u.registerComponent<TestComponent<0>>(), 0u);
        TC_RequireEqual(u.registerComponent<TestComponent<1>>(), 1u);
        TC_RequireEqual(u.registerComponent<TestComponent<2>>(), 2u);
        TC_RequireEqual(u.componentMask<TestComponent<0>>(), ent::ComponentBitset().set(0));
        TC_RequireEqual(u.componentMask<TestComponent<1>>(), ent::ComponentBitset().set(1));
        TC_RequireEqual(u.componentMask<TestComponent<2>>(), ent::ComponentBitset().set(2));

        for (u32 iii = 0; iii < 100; ++iii)
        {
            ent::EntityId id(iii);

            TC_Require(!u.hasComponent<TestComponent<0>>(id));
            TC_Require(!u.hasComponent<TestComponent<1>>(id));
            TC_Require(!u.hasComponent<TestComponent<2>>(id));
            auto a{u.getComponent<TestComponent<0>>(id)};
            UNUSED(a);
            TC_Require(!u.getComponent<TestComponent<0>>(id));
            TC_Require(!u.getComponent<TestComponent<1>>(id));
            TC_Require(!u.getComponent<TestComponent<2>>(id));
            u.removeComponent<TestComponent<0>>(id);
            u.removeComponent<TestComponent<1>>(id);
            u.removeComponent<TestComponent<2>>(id);

            TestComponent<1> *ptr1{u.addComponent<TestComponent<1>>(id)};
            TC_Require(ptr1);
            TC_RequireEqual(ptr1, u.getComponent<TestComponent<1>>(id));
            TestComponent<2> *ptr2{u.addComponent<TestComponent<2>>(id)};
            TC_Require(ptr2);
            TC_RequireEqual(ptr2, u.addComponent<TestComponent<2>>(id));

            ptr1->v = 42u;
            TC_RequireEqual(u.getComponent<TestComponent<1>>(id)->v, 42u);

            TC_Require(!u.hasComponent<TestComponent<0>>(id));
            TC_Require(u.hasComponent<TestComponent<1>>(id));
            TC_Require(u.hasComponent<TestComponent<2>>(id));
            TC_Require(!u.getComponent<TestComponent<0>>(id));
            TC_Require(u.getComponent<TestComponent<1>>(id));
            TC_Require(u.getComponent<TestComponent<2>>(id));
            u.removeComponent<TestComponent<0>>(id);
            u.removeComponent<TestComponent<1>>(id);
            u.removeComponent<TestComponent<2>>(id);

            TC_Require(!u.hasComponent<TestComponent<0>>(id));
            TC_Require(!u.hasComponent<TestComponent<1>>(id));
            TC_Require(!u.hasComponent<TestComponent<2>>(id));
            TC_Require(!u.getComponent<TestComponent<0>>(id));
            TC_Require(!u.getComponent<TestComponent<1>>(id));
            TC_Require(!u.getComponent<TestComponent<2>>(id));
        }
    }

    TU_Case(ComponentManager1, "Testing the ComponentHolder class")
    {
        PROF_SCOPE("ComponentManager1");
        SecondUniverse::UniverseT &u{SecondUniverse::instance()};
        using Entity = SecondUniverse::EntityT;
        TC_RequireEqual(u.componentMask<TestComponent<0>>(), ent::ComponentBitset().set(0));
        TC_RequireEqual(u.componentMask<TestComponent<1>>(), ent::ComponentBitset().set(1));
        TC_RequireEqual(u.componentMask<TestComponent<2>>(), ent::ComponentBitset().set(2));

        for (u32 iii = 0; iii < 100; ++iii)
        {
            //Entity ent(&u, iii);
            Entity ent = u.createEntity();

            TC_Require(!ent.has<TestComponent<0>>());
            TC_Require(!ent.has<TestComponent<1>>());
            TC_Require(!ent.has<TestComponent<2>>());
            auto a{ent.get<TestComponent<0>>()};
            UNUSED(a);
            TC_Require(!ent.get<TestComponent<0>>());
            TC_Require(!ent.get<TestComponent<1>>());
            TC_Require(!ent.get<TestComponent<2>>());
            ent.remove<TestComponent<0>>();
            ent.remove<TestComponent<1>>();
            ent.remove<TestComponent<2>>();

            TestComponent<1> *ptr1{ent.add<TestComponent<1>>()};
            TC_Require(ptr1);
            TC_RequireEqual(ptr1, ent.get<TestComponent<1>>());
            TestComponent<2> *ptr2{ent.add<TestComponent<2>>()};
            TC_Require(ptr2);
            TC_RequireEqual(ptr2, ent.get<TestComponent<2>>());

            ptr1->v = 42u;
            TC_RequireEqual(ent.get<TestComponent<1>>()->v, 42u);

            TC_Require(!ent.has<TestComponent<0>>());
            TC_Require(ent.has<TestComponent<1>>());
            TC_Require(ent.has<TestComponent<2>>());
            TC_Require(!ent.get<TestComponent<0>>());
            TC_Require(ent.get<TestComponent<1>>());
            TC_Require(ent.get<TestComponent<2>>());
            ent.remove<TestComponent<0>>();
            ent.remove<TestComponent<1>>();
            ent.remove<TestComponent<2>>();

            TC_Require(!ent.has<TestComponent<0>>());
            TC_Require(!ent.has<TestComponent<1>>());
            TC_Require(!ent.has<TestComponent<2>>());
            TC_Require(!ent.get<TestComponent<0>>());
            TC_Require(!ent.get<TestComponent<1>>());
            TC_Require(!ent.get<TestComponent<2>>());
        }
    }

    TU_Case(SystemManager0, "Testing the SystemManager class")
    {
        PROF_SCOPE("SystemManager0");
        SecondUniverse::UniverseT &u{SecondUniverse::instance()};

        TestSystem *sys1 = u.addSystem<TestSystem>(1u);
        TestSystem2<0> *sys2 = u.addSystem<TestSystem2<0>>(2u);
        TestSystem2<1> *sys3 = u.addSystem<TestSystem2<1>>(3u);

        TC_Require(sys1);
        TC_Require(sys2);
        TC_Require(sys3);

        TC_Require(sys1->mNum == 1u);
        TC_Require(sys2->mNum == 2u);
        TC_Require(sys3->mNum == 3u);

        TC_RequireEqual(sys1->getFilter(), ent::ComponentFilter({}, {}));
        TC_RequireEqual(sys2->getFilter(), ent::ComponentFilter(1u, 2u));
        TC_RequireEqual(sys3->getFilter(), ent::ComponentFilter(1u, 2u));

        TC_RequireEqual(sys1->getGroupId(), 0u);
        TC_RequireEqual(sys2->getGroupId(), 1u);
        TC_RequireEqual(sys3->getGroupId(), 1u);
    }

    TU_Case(ComplexTest0, "Testing Universe initialization")
    {
        RealUniverse1::UniverseT &u{RealUniverse1::instance()};

        TC_RequireEqual(u.registerComponent<TestComponent<0>>(), 0u);
        TC_RequireEqual(u.registerComponent<TestComponent<1>>(), 1u);
        TC_RequireEqual(u.registerComponent<TestComponent<2>>(), 2u);

        TC_RequireEqual(u.componentMask<TestComponent<0>>(), ent::ComponentBitset().set(0));
        TC_RequireEqual(u.componentMask<TestComponent<1>>(), ent::ComponentBitset().set(1));
        TC_RequireEqual(u.componentMask<TestComponent<2>>(), ent::ComponentBitset().set(2));

        TestSystem *sys1 = u.addSystem<TestSystem>(1u);
        TestSystem2<0> *sys2 = u.addSystem<TestSystem2<0>>(2u);
        TestSystem2<1> *sys3 = u.addSystem<TestSystem2<1>>(3u);

        TC_Require(sys1);
        TC_Require(sys2);
        TC_Require(sys3);

        u.init();

        u.refresh();
    }

TU_End(EntropyEntity)

int main(int argc, char* argv[])
{
    TCC_Run();
    TCC_Report();

    prof::PrintCrawler pc;
    PROF_DUMP(pc);

    return TCC_ReturnCode;
}

