/**
 * @file test/Tests.cpp
 * @author Tomas Polasek
 * @brief Main testing file.
 */

#include "Tests.h"

template <typename T>
class ClassIdGenerator
{
public:
    template <typename ClassT>
    static constexpr u64 getId()
    { return mId<ClassT>; }
private:
    template <typename ClassT>
    struct Tag
    {
        constexpr Tag() {}
    };
    struct ClassIdHolder
    {
        /*
        template <u64 N>
        struct Flag;
        template <u64 N>
        static constexpr u64 cId(Flag<N>);
         */

        template <u64 N, u64 = N>
        struct Flag
        {
            template <u64 M>
            friend constexpr u64 cId(Flag<N, M>);
        };

        template <u64 N>
        struct SpecHolder
        {
            template <u64 M>
            friend constexpr u64 cId(Flag<N, M>)
            { return N; }
            static constexpr u64 value{N};
        };

        /*
        template <u64 N>
        static constexpr SpecHolder<N> mark(u64 val = SpecHolder<N>::value)
        { return SpecHolder<N>{}; }
         */

        template <u64 N>
        static constexpr u64 mark(u64 val = SpecHolder<N>::value)
        { return val; }

        //static_assert(sizeof(SpecHolder<0>));
        /*
        static_assert(sizeof(mark<0>()));
        static_assert(noexcept(cId(Flag<0>{})));
        static_assert(sizeof(mark<1>()));
        static_assert(noexcept(cId(Flag<1>{})));
        struct Test
        {
            static_assert(sizeof(mark<2>()));
            static_assert(noexcept(cId(Flag<2>{})));
        } t;
         */

        /*
        template <u64 N>
        struct Marker
        {
            static_assert(sizeof(mark<N>()));
            static_assert(noexcept(cId(Flag<N>{})));

            static constexpr u64 value{N};
        };
         */

        /*
        template <u64 N,
                  typename Enable = void>
        struct Reader
        {
            //static constexpr auto _mark{mark<N>()};
            static_assert(!noexcept(cId(Flag<N>{})));
            static constexpr Marker<N> _m{};
            static_assert(noexcept(cId(Flag<N>{})));
            static constexpr u64 value{N};
        };

        template <u64 N>
        struct Reader <N, typename std::enable_if<noexcept(cId(Flag<N>{}))>::type>
        {
            static_assert(!noexcept(cId(Flag<3>{})));
            static constexpr u64 value{Reader<N + 1>::value};
        };
         */

        template <u64 N,
                  typename Enable = typename std::enable_if<!noexcept(cId(Flag<N>{}))>::type>
        static constexpr u64 reader(int, Flag<N> = {})
        { static_assert(!noexcept(cId(Flag<N>{}))); return N; }

        template <u64 N>
        static constexpr u64 reader(float, Flag<N> = {}, u64 val = reader<N + 1>(0))
        { return val; }

        //static constexpr u64 next(u64 val = Reader<0>::value)
        template <u64 V = mark<reader<0>(0)>()>
        static constexpr u64 next()
        {
            return V;
        }

        static constexpr void fun()
        {
            static_assert(noexcept(cId(Flag<3>{})));
        }
    };

    template <typename ClassT>
    static constexpr u64 mId{ClassIdHolder::next()};
protected:
};

/*
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
 */

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

/*
template <u64 N>
struct flag {
    friend constexpr u64 adl_flag(flag<N>);
};

template <u64 N>
struct writer {
    friend constexpr u64 adl_flag(flag<N>) {
        return N;
    }

    static constexpr u64 value{N};
};

template <u64 N, u64 = adl_flag(flag<N>{})>
constexpr u64 reader(int, flag<N>) {
    return N;
}

template <u64 N>
constexpr u64 reader(float, flag<N>, u64 R = reader(0, flag<N - 1> {})) {
    return R;
}

constexpr u64 reader(float, flag<0>) {
    return 0;
}

template <u64 N = 1>
constexpr u64 next(u64 R = writer<reader(0, flag<32> {}) + N>::value) {
    return R;
}
 */

TU_Begin(MiscUnit)

    TU_Setup
    {

    }

    TU_Teardown
    {

    };

    TU_Case(Misc0, "Miscellaneous tests")
    {
        using Gen1 = ClassIdGenerator<int>;
        using Gen2 = ClassIdGenerator<double>;

        // Generator 1
        TC_RequireConstexpr(Gen1::getId<int>() == 0);
        TC_RequireConstexpr(Gen1::getId<double>() == 1);
        TC_RequireConstexpr(Gen1::getId<float>() == 2);
        TC_RequireConstexpr(Gen1::getId<long>() == 3);
        TC_RequireConstexpr(Gen1::getId<int>() == 0);

        // Generator 2
        TC_RequireConstexpr(Gen2::getId<double>() == 0);
        TC_RequireConstexpr(Gen2::getId<int>() == 1);
        TC_RequireConstexpr(Gen2::getId<float>() == 2);
        TC_RequireConstexpr(Gen2::getId<long>() == 3);
        TC_RequireConstexpr(Gen2::getId<double>() == 0);
    }

TU_End(MiscUnit)

int main(int argc, char* argv[])
{
    prof::PrintCrawler pc;
    PROF_DUMP(pc);

    TCC_Run();
    TCC_Report();

    return TCC_ReturnCode;
}
