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



TU_Begin(EntropyEntity)

    TU_Setup
    {

    }

    TU_Teardown
    {

    };

    TU_Case(ClassIdGenerator0, "Testing the ClassIdGenerator class")
    {
        class GenA : public ent::ClassIdGenerator<GenA> {};
        TC_RequireConstexprEqual(GenA::getId<u32>(), 0);
        TC_RequireConstexprEqual(GenA::getId<u64>(), 1);
        TC_RequireConstexprEqual(GenA::getId<double>(), 2);
        TC_RequireConstexprEqual(GenA::getId<float>(), 3);
        TC_RequireConstexprEqual(GenA::getId<u32>(), 0);
        TC_RequireConstexprEqual(GenA::getId<u64>(), 1);

        class GenB : public ent::ClassIdGenerator<GenB> {};
        TC_RequireConstexprEqual(GenB::getId<double>(), 0);
        TC_RequireConstexprEqual(GenB::getId<float>(), 1);
        TC_RequireConstexprEqual(GenB::getId<u32>(), 2);
        TC_RequireConstexprEqual(GenB::getId<u64>(), 3);
        TC_RequireConstexprEqual(GenB::getId<double>(), 0);
        TC_RequireConstexprEqual(GenB::getId<float>(), 1);

        class GenC : public ent::ClassIdGenerator<GenC, 5> {};
        TC_RequireConstexprEqual(GenC::getId<double>(), 5);
        TC_RequireConstexprEqual(GenC::getId<float>(), 6);
        TC_RequireConstexprEqual(GenC::getId<u32>(), 7);
        TC_RequireConstexprEqual(GenC::getId<u64>(), 8);
        TC_RequireConstexprEqual(GenC::getId<double>(), 5);
        TC_RequireConstexprEqual(GenC::getId<float>(), 6);
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
    prof::PrintCrawler pc;
    PROF_DUMP(pc);

    TCC_Run();
    TCC_Report();

    return TCC_ReturnCode;
}

