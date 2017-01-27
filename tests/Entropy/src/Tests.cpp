/**
 * @file test/Tests.cpp
 * @author Tomas Polasek
 * @brief Main testing file.
 */

#include "Tests.h"

class TestSystem: public ent::System
{
public:
    TestSystem(u32 num) :
        mNum{num}
    { }
    u32 mNum;
};


template <typename A>
class ClassA
{
public:
    class ClassB
    {
    public:
        template <typename T>
        static constexpr u64 a{0};
    };

    static constexpr auto oc{ClassB::template a<ClassB>};
    static constexpr auto c{&ClassB::template a<int> - &ClassB::template a<ClassB>};
    //static constexpr u64 c{&ClassB::template a<int> - &ClassB::template a<double>};
};

/*
template <typename A>
class MemberOrderTest
{
public:
    template <typename B>
    class Order
    {
    public:
        template <typename T>
        static u8 beg;
        template <typename T>
        static u8 pos;
    };

    template <typename T>
    static constexpr auto val{&Order<A>::pos<T>};
private:
protected:
};

template <typename T>
u8 MemberOrderTest::Order::pos<T>{0};

template <typename T>
u8 MemberOrderTest::Order::beg<T>{0};
 */

TU_Begin(EntropyEntity)

    TU_Setup
    {

    }

    TU_Teardown
    {

    };

    TU_Case(ClassIdGenerator0, "Testing the ClassIdGenerator class")
    {
        /*
        class GenA : public ent::ClassIdGenerator<GenA> {};
        TC_Require(GenA::getId<u32>() == 0);
        TC_Require(GenA::getId<u64>() == 1);
        TC_Require(GenA::getId<double>() == 2);
        TC_Require(GenA::getId<float>() == 3);
        TC_Require(GenA::getId<u32>() == 0);
        TC_Require(GenA::getId<u64>() == 1);
         */
    }

    TU_Case(Universe0, "Testing the Universe class")
    {
        ent::Universe<0> u;
        TC_Require(u.addSystem<TestSystem>(1).mNum == 1);
        TC_RequireNoException(u.removeSystem<TestSystem>());
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

TU_End(EntropyEntity)

int main(int argc, char* argv[])
{
    /*
    std::cout << "u32: " << (u64)&MemberOrderTest::Order::beg<MemberOrderTest::Order> << "." << std::endl;
    std::cout << "u32: " << (u64)MemberOrderTest::val<u32> << "." << std::endl;
    std::cout << "u32: " << (u64)MemberOrderTest::val<u32> << "." << std::endl;
    std::cout << "u64: " << (u64)MemberOrderTest::val<u64> << "." << std::endl;
    std::cout << "u64: " << (u64)MemberOrderTest::val<u64> << "." << std::endl;
     */

    std::cout << ClassA<int>::c << std::endl;

    prof::PrintCrawler pc;
    PROF_DUMP(pc);

    TCC_Run();
    TCC_Report();

    return TCC_ReturnCode;
}

