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
template <typename ComponentT,
    typename NUM>
u32 TestComponentHolder<ComponentT, NUM>::mNum{0};

class FirstTestSystem: public FirstUniverse::SystemT
{
public:
    FirstTestSystem(u32 num) :
        mNum{num} {}

    const auto &getFilter() const
    { return filter(); }

    auto getGroupId() const
    { return groupId(); }

    u32 mNum;
};

class TestSystem: public SecondUniverse::SystemT
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

class RealTestSystem: public RealUniverse1::SystemT
{
public:
    RealTestSystem(u32 num) :
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
class TestSystem2: public SecondUniverse::SystemT
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

template <u64 N>
class RealTestSystem2: public RealUniverse1::SystemT
{
public:
    using Require = ent::Require<TestComponent<0>>;
    using Reject = ent::Reject<TestComponent<1>>;

    RealTestSystem2(u32 num) :
        mNum{num} {}

    const auto &getFilter() const
    { return filter(); }

    auto getGroupId() const
    { return groupId(); }

    u32 mNum;
};

struct Velocity
{
    float x;
    float y;

    friend std::ostream &operator<<(std::ostream &out, const Velocity &vel)
    {
        out << "Vel: " << vel.x << ", " << vel.y;
        return out;
    }
};

struct Position
{
    float x;
    float y;

    friend std::ostream &operator<<(std::ostream &out, const Position &pos)
    {
        out << "Pos: " << pos.x << ", " << pos.y;
        return out;
    }
};

class MovementSystem : public RealUniverse2::SystemT
{
public:
    using Require = ent::Require<Velocity, Position>;

    void run()
    {
        std::cout << "Running movement system!" << std::endl;

        if (isInitialized())
        {
            std::cout << "System IS initialized!" << std::endl;

            std::cout << "Added " << foreachAdded().size() << " Entities." << std::endl;
            for (auto &e : foreachAdded())
            {
                std::cout << "Added Entity : " << *e.get<Position>() << " " << *e.get<Velocity>() << std::endl;
            }

            std::cout << "Removed " << foreachRemoved().size() << " Entities." << std::endl;
            for (auto &e : foreachRemoved())
            {
                std::cout << "Removed Entity : " << *e.get<Position>() << " " << *e.get<Velocity>() << std::endl;
            }

            std::cout << "Iterating over " << foreach().size() << " Entities." << std::endl;
            for (auto &e : foreach())
            {
                std::cout << "Entity : " << *e.get<Position>() << " " << *e.get<Velocity>() << std::endl;
            }
        }
        else
        {
            std::cout << "System is NOT initialized yet!" << std::endl;
        }
    }
};

template <typename T>
struct DestructionHolder : public ent::BaseComponentHolder<T>
{
    DestructionHolder()
    {
        sConstructed++;
    }

    ~DestructionHolder()
    {
        sDestructed++;
    }

    virtual T* add(ent::EntityId id) noexcept override
    {
        return &mInst;
    }

    virtual T* get(ent::EntityId id) noexcept override
    {
        return &mInst;
    }

    virtual const T* get(ent::EntityId id) const noexcept override
    {
        return &mInst;
    }

    virtual bool remove(ent::EntityId id) noexcept override
    {
        return true;
    }

    virtual void refresh() noexcept override
    { }

    T mInst;

    static u64 sConstructed;
    static u64 sDestructed;
};

template <typename T>
u64 DestructionHolder<T>::sConstructed{0};
template <typename T>
u64 DestructionHolder<T>::sDestructed{0};

struct DestructionC
{
    using HolderT = DestructionHolder<DestructionC>;

    float x;
};

struct DestructionSystem : public RealUniverse3::SystemT
{
    using Require = ent::Require<DestructionC>;

    DestructionSystem()
    {
        sConstructed++;
    }

    ~DestructionSystem()
    {
        sDestructed++;
    }

    static u64 sConstructed;
    static u64 sDestructed;
};

u64 DestructionSystem::sConstructed{0};
u64 DestructionSystem::sDestructed{0};

TU_Begin(EntropyEntity)

    TU_Setup
    {

    }

    TU_Teardown
    {

    }

	/*
    TU_Case(ClassIdGenerator0, "Testing the ClassIdGenerator class")
    {
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
	*/

    TU_Case(Universe0, "Testing the Universe class")
    {
        using Universe = FirstUniverse;
        Universe::UniverseT &u{Universe::instance()};
        TC_Require(u.addSystem<FirstTestSystem>(1)->mNum == 1);
        TC_RequireNoException(u.removeSystem<FirstTestSystem>());
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
        ent::EntityId eid;
        TC_Check(eid.id() == 0);
        TC_Check(eid.index() == 0);
        TC_Check(eid.generation() == 0);
    }

    TU_Case(EntityId1, "Testing the EntityId class")
    {
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
        using EntityT1 = FirstUniverse::EntityT;
        using EntityT2 = SecondUniverse::EntityT;
        FirstUniverse::UniverseT *un1{reinterpret_cast<FirstUniverse::UniverseT*>(1u)};
        SecondUniverse::UniverseT *un2{reinterpret_cast<SecondUniverse::UniverseT*>(2u)};
        EntityT1 ent11(un1, 1);
        EntityT2 ent21(un2, 2);
        TC_Require(ent11.universe() == un1 && ent11.id().index() == 1);
        TC_Require(ent21.universe() == un2 && ent21.id().index() == 2);
        EntityT1 ent12(ent11);
        EntityT2 ent22(ent21);
        TC_Require(ent12.universe() == un1 && ent12.id().index() == 1);
        TC_Require(ent22.universe() == un2 && ent22.id().index() == 2);
        TC_Require(ent12 == ent11);
        TC_Require(ent22 == ent21);
        ent12 = ent21;
        ent22 = ent11;
        TC_Require(ent12.universe() == un1 && ent12.id().index() == 2);
        TC_Require(ent22.universe() == un2 && ent22.id().index() == 1);
    }

    TU_Case(EntityHolder0, "Testing the EntityHolder class")
    {
        using ent::EntityHolder;
        using ent::EntityId;
        static constexpr u64 CREATE_NUM{100};
        EntityHolder h1;

        for (u64 iii = 1; iii <= CREATE_NUM; ++iii)
        {
            TC_RequireEqual(h1.create(), EntityId(iii, 0));
            TC_Require(h1.active(EntityId(iii, 0)));
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
        SecondUniverse::UniverseT &u{SecondUniverse::instance()};
        using Entity = SecondUniverse::EntityT;
        TC_RequireEqual(u.registerComponent<TestComponent<0>>(), 0u);
        TC_RequireEqual(u.registerComponent<TestComponent<1>>(), 1u);
        TC_RequireEqual(u.registerComponent<TestComponent<2>>(), 2u);
        TC_RequireEqual(u.componentMask<TestComponent<0>>(), 1u);
        TC_RequireEqual(u.componentMask<TestComponent<1>>(), 2u);
        TC_RequireEqual(u.componentMask<TestComponent<2>>(), 4u);

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

        u.reset();
        u.init();

        for (u64 iii = 0; iii < 100; ++iii)
        {
            Entity e = u.createEntity();

            TC_Require(e.id().index() <= ent::ENT_MIN_FREE);

            e.destroy();
        }
    }

    TU_Case(SystemManager0, "Testing the SystemManager class")
    {
        SecondUniverse::UniverseT &u{SecondUniverse::instance()};
        TC_RequireEqual(u.registerComponent<TestComponent<0>>(), 0u);
        TC_RequireEqual(u.registerComponent<TestComponent<1>>(), 1u);
        TC_RequireEqual(u.registerComponent<TestComponent<2>>(), 2u);
        TC_RequireEqual(u.componentMask<TestComponent<0>>(), 1u);
        TC_RequireEqual(u.componentMask<TestComponent<1>>(), 2u);
        TC_RequireEqual(u.componentMask<TestComponent<2>>(), 4u);

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

        TC_RequireEqual(u.componentMask<TestComponent<0>>(), 1u);
        TC_RequireEqual(u.componentMask<TestComponent<1>>(), 2u);
        TC_RequireEqual(u.componentMask<TestComponent<2>>(), 4u);

        RealTestSystem *sys1 = u.addSystem<RealTestSystem>(1u);
        RealTestSystem2<0> *sys2 = u.addSystem<RealTestSystem2<0>>(2u);
        RealTestSystem2<1> *sys3 = u.addSystem<RealTestSystem2<1>>(3u);

        TC_Require(sys1);
        TC_Require(sys2);
        TC_Require(sys3);

        u.init();

        u.refresh();
    }

    TU_Case(ComplexTest1, "Testing System iteration")
    {
        static constexpr u64 NUM_ENTITIES{1000u};
        static constexpr u64 NUM_ITERATIONS{10u};
        using Universe = RealUniverse2::UniverseT;
        using Entity = RealUniverse2::EntityT;

        Universe &u{RealUniverse2::instance()};

        TC_RequireEqual(u.registerComponent<Position>(), 0u);
        TC_RequireEqual(u.registerComponent<Velocity>(), 1u);

        MovementSystem *sys{u.addSystem<MovementSystem>()};
        TC_Require(sys);
        TC_Require(sys->isInitialized());

        u.init();

        TC_Require(sys->foreach().size() == 0);

        u64 lastSysEnt{0u};
        u64 sysEnt{0u};
        u64 sysAdded{0u};
        u64 sysRemoved{0u};

        for (u64 it = 1; it <= NUM_ITERATIONS; ++it)
        {
            lastSysEnt = sysEnt;
            sysRemoved = 0u;
            sysAdded = 0u;

            for (u64 iii = 0; iii < NUM_ENTITIES; ++iii)
            {
                PROF_BLOCK("Creating Entities");
                Entity e = u.createEntity();
                PROF_BLOCK_END();
                TC_Require(e.id().index());
                PROF_BLOCK("Adding Components");
                sysAdded++;
                sysEnt++;
                e.add<Position>();
                e.add<Velocity>();
                PROF_BLOCK_END();
            }

            TC_Require(sys->foreach().size() == lastSysEnt);

            PROF_BLOCK("Refresh after adding Entities");
            u.refresh();
            PROF_BLOCK_END();

            TC_RequireEqual(sys->foreachAdded().size(), sysAdded);
            TC_RequireEqual(sys->foreachRemoved().size(), sysRemoved);
            TC_RequireEqual(sys->foreach().size(), sysEnt);

            sysRemoved = 0u;
            sysAdded = 0u;

            PROF_BLOCK("Iteration over Entities");
            for (auto &e : sys->foreach())
            {
                if (e.has<Position>() && e.has<Velocity>())
                {
                    sysRemoved++;
                    sysEnt--;
                    e.remove<Position>();
                    e.remove<Velocity>();
                }
                else
                {
                    sysAdded++;
                    sysEnt++;
                    e.add<Position>();
                    e.add<Velocity>();
                }
            }
            PROF_BLOCK_END();

            u.refresh();

            TC_RequireEqual(sys->foreachAdded().size(), sysAdded);
            TC_RequireEqual(sys->foreachRemoved().size(), sysRemoved);
            TC_RequireEqual(sys->foreach().size(), sysEnt);
        }

        TC_RequireEqual((u.addGetGroup<ent::Require<>, ent::Reject<>>()->foreach(&u).size()), 0u);
        TC_RequireEqual((u.addGetGroup<ent::Require<Position>, ent::Reject<>>()->foreach(&u).size()), 0u);

        u.refresh();

        TC_RequireEqual((u.addGetGroup<ent::Require<>, ent::Reject<>>()->foreach(&u).size()), sysRemoved);
        TC_RequireEqual((u.addGetGroup<ent::Require<Position>, ent::Reject<>>()->foreach(&u).size()), sysEnt);

        for (auto &e : u.addGetGroup<ent::Require<>, ent::Reject<>>()->foreach(&u))
        {
            e.destroy();
        }

        for (auto &e : u.addGetGroup<ent::Require<Position>, ent::Reject<>>()->foreach(&u))
        {
            e.destroy();
        }

        u.refresh();

        TC_RequireEqual((u.addGetGroup<ent::Require<>, ent::Reject<>>()->foreach(&u).size()), 0u);
        TC_RequireEqual((u.addGetGroup<ent::Require<Position>, ent::Reject<>>()->foreach(&u).size()), 0u);
    }

    TU_Case(UniverseDestruction, "Testing correct destruction of Universe instance.")
    {
        using Entity = RealUniverse3::EntityT;
        using Holder = DestructionHolder<DestructionC>;
        DestructionSystem *sys1{nullptr};
        DestructionSystem *sys2{nullptr};

        {
            RealUniverse3 u;

            TC_RequireEqual(u.registerComponent<DestructionC>(), 0u);

            TC_RequireEqual(Holder::sConstructed, 1u);
            TC_RequireEqual(Holder::sDestructed, 0u);

            sys1 = u.addSystem<DestructionSystem>();

            TC_RequireEqual(DestructionSystem::sConstructed, 1u);
            TC_RequireEqual(DestructionSystem::sDestructed, 0u);

            u.init();

            TC_Require(sys1->isInitialized());
            TC_RequireEqual(sys1->foreach().size(), 0u);

            Entity e = u.createEntity();

            TC_RequireEqual(e.id(), ent::EntityId(1u, 0u));

            e.add<DestructionC>();

            u.refresh();

            TC_Require(sys1->isInitialized());
            TC_RequireEqual(sys1->foreach().size(), 1u);
        }

        TC_RequireEqual(Holder::sConstructed, 1u);
        TC_RequireEqual(Holder::sDestructed, 1u);
        TC_RequireEqual(DestructionSystem::sConstructed, 1u);
        TC_RequireEqual(DestructionSystem::sDestructed, 1u);

        {
            RealUniverse3 u;

            TC_RequireEqual(u.registerComponent<DestructionC>(), 0u);

            TC_RequireEqual(Holder::sConstructed, 2u);
            TC_RequireEqual(Holder::sDestructed, 1u);

            sys2 = u.addSystem<DestructionSystem>();

            TC_RequireEqual(DestructionSystem::sConstructed, 2u);
            TC_RequireEqual(DestructionSystem::sDestructed, 1u);

            u.init();

            TC_Require(sys2->isInitialized());
            TC_RequireEqual(sys1, sys2);
            TC_RequireEqual(sys2->foreach().size(), 0u);

            Entity e = u.createEntity();

            TC_RequireEqual(e.id(), ent::EntityId(1u, 0u));

            e.add<DestructionC>();

            u.refresh();

            TC_Require(sys2->isInitialized());
            TC_RequireEqual(sys2->foreach().size(), 1u);
        }

        TC_RequireEqual(Holder::sConstructed, 2u);
        TC_RequireEqual(Holder::sDestructed, 2u);
        TC_RequireEqual(DestructionSystem::sConstructed, 2u);
        TC_RequireEqual(DestructionSystem::sDestructed, 2u);

        {
            RealUniverse3 u;
            TC_RequireEqual(u.registerComponent<DestructionC>(), 0u);
            TC_RequireEqual(u.addSystem<DestructionSystem>(), sys1);
            u.reset();
            TC_RequireEqual(u.registerComponent<DestructionC>(), 0u);
            TC_RequireEqual(u.addSystem<DestructionSystem>(), sys1);
            u.reset();
            TC_RequireEqual(u.registerComponent<DestructionC>(), 0u);
            TC_RequireEqual(u.addSystem<DestructionSystem>(), sys1);
        }
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

