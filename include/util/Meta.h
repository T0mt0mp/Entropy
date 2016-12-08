/**
 * @file util/Meta.h
 * @author Tomas Polasek
 * @brief Meta programming utilities.
 */

#ifndef UTIL_META_H
#define UTIL_META_H

#include <type_traits>

#include "util/Types.h"

template <typename ...>
using mp_void_t = void;

/// Type list
template <typename... Ts> struct mp_list{};

/// Rename implementation
template <typename A,
          template <typename...> typename B>
struct mp_rename_impl;

template <template <typename...> typename A,
          typename... Types,
          template <typename...> typename B>
struct mp_rename_impl<A<Types...>, B>
{
    using type = B<Types...>;
};

template <template <typename...> typename,
          typename...>
struct mp_are_specializations_impl : std::false_type
{};

template <template <typename...> typename Base,
          typename... Ts>
struct mp_are_specializations_impl<Base, Base<Ts...>> : std::true_type
{};

template <template <typename...> typename Base,
          typename... Ts,
          typename Next,
          typename... Rest>
struct mp_are_specializations_impl<Base, Base<Ts...>, Next, Rest...>
{
    static constexpr bool value =
        mp_are_specializations_impl<Base, Next, Rest...>::value;
};

template <template <typename...> typename Fun,
          typename ArgHolder,
          typename... Arguments>
struct mp_call_on_arguments_impl;

template <template <typename...> typename Fun,
          template <typename...> typename ArgHolder,
          typename... ArgHolderArgs,
          typename... Arguments>
struct mp_call_on_arguments_impl<Fun, ArgHolder<ArgHolderArgs...>, Arguments...> :
    Fun<Arguments..., ArgHolderArgs...>
{};

template <typename Element,
          typename List>
struct mp_contains_impl : std::false_type
{};

template <typename Element,
          template <typename...> typename List,
          typename... Rest>
struct mp_contains_impl<Element, List<Element, Rest...>> : std::true_type
{};

template <typename Element,
          template <typename...> typename List,
          typename First,
          typename... Rest>
struct mp_contains_impl<Element, List<First, Rest...>>
{
    static constexpr bool value = mp_contains_impl<Element, List<Rest...>>::value;
};

template <typename List>
struct mp_size_impl :
    std::integral_constant<u64, 0>
{};

template <template <typename...> typename List,
          typename... Content>
struct mp_size_impl<List<Content...>> :
    std::integral_constant<u64, sizeof...(Content)>
{};

template <typename... Args>
struct mp_length_impl :
    std::integral_constant<u64, sizeof...(Args)>
{};

template <typename Element,
          typename List>
struct mp_index_of_impl;

template <typename Element,
    template <typename...> typename List,
    typename... Rest>
struct mp_index_of_impl<Element, List<Element, Rest...>> :
    std::integral_constant<u64, 0>
{
};

template <typename Element,
          template <typename...> typename List,
          typename First,
          typename... Rest>
struct mp_index_of_impl<Element, List<First, Rest...>> :
    std::integral_constant<u64, mp_index_of_impl<Element, List<Rest...>>::value + 1>
{
};

template <typename Element,
          template <typename...> typename List>
struct mp_index_of_impl<Element, List<>> :
    std::integral_constant<u64, 0>
{
    /// When the element is not found, this message is printed.
    /// Has to depend on Element.
    static_assert(sizeof(Element) == 0, "Element not found!");
};

template <bool Cond,
          typename Type>
struct mp_fail_on_false_impl;

template <typename Type>
struct mp_fail_on_false_impl<true, Type>
{
    using type = Type;
};

template <template <typename...> typename Fun,
          typename... Params>
struct mp_bind_impl
{
    // Inner bound function.
    template <typename... CallParams>
    struct type : Fun<Params..., CallParams...>
    {};
};

template <template <typename...> typename Fun,
          typename... Lists>
struct mp_transform_impl;

template <template <typename...> typename Fun,
          template <typename...> typename L1,
          typename... L1Params>
struct mp_transform_impl<Fun, L1<L1Params...>>
{
    using type = L1<Fun<L1Params>...>;
};

template <template <typename...> typename Fun,
          template <typename...> typename L1,
          typename... L1Params,
          template <typename...> typename L2,
          typename... L2Params>
struct mp_transform_impl<Fun, L1<L1Params...>, L2<L2Params...>>
{
    static_assert(sizeof...(L1Params) == sizeof...(L2Params),
                  "Parameter packs have to be the same size.");
    using type = L1<Fun<L1Params, L1Params>...>;
};

template <typename List>
struct mp_list_empty_impl : std::true_type
{};

template <template <typename...> typename List,
          typename... Pack>
struct mp_list_empty_impl<List<Pack...>> : std::false_type
{};

template <typename... Pack>
struct mp_empty_impl : std::integral_constant<bool, !sizeof...(Pack)>
{};

template <u64 N,
          typename List>
struct mp_get_nth_impl;

template <template <typename...> typename List,
          typename First,
          typename... Elements>
struct mp_get_nth_impl<0, List<First, Elements...>>
{
    using type = First;
};

template <u64 N,
          template <typename...> typename List,
          typename First,
          typename... Elements>
struct mp_get_nth_impl<N, List<First, Elements...>>
{
    static_assert(N <= sizeof...(Elements),
                  "N is larger than number of elements");
    using type = typename mp_get_nth_impl<N - 1, List<Elements...>>::type;
};

template <typename Op,
          typename List>
struct mp_accumulate_impl;

template <typename Op,
          template <typename...> typename List,
          typename Last>
struct mp_accumulate_impl<Op, List<Last>>
{
    static constexpr auto value = Last::value;
};

template <typename Op,
          template <typename...> typename List,
          typename First,
          typename... Rest>
struct mp_accumulate_impl<Op, List<First, Rest...>>
{
    template <typename T>
    static constexpr auto _accumulate(T a, T b)
    {
        constexpr Op o;
        return o(a, b);
    }

    using _RestResult = mp_accumulate_impl<Op, List<Rest...>>;

    static constexpr auto value = _accumulate(First::value, _RestResult::value);
};

template <typename T>
struct mp_remove_const_impl
{
    using type = T;
};

template <typename T>
struct mp_remove_const_impl<const T>
{
    using type = T;
};

template <bool flag,
          typename TrueOut,
          typename FalseOut>
struct mp_choose_impl
{
    using type = FalseOut;
};

template <typename TrueOut,
          typename FalseOut>
struct mp_choose_impl<true, TrueOut, FalseOut>
{
    using type = TrueOut;
};

template <typename FirstT,
          typename SecondT>
struct mp_is_same_impl
{
    static constexpr bool value{false};
};

template <typename FirstT>
struct mp_is_same_impl<FirstT, FirstT>
{
    static constexpr bool value{true};
};

template <typename T,
          u64 SIZE = sizeof(T)>
struct mp_memory_filler_impl;

template <typename T>
struct mp_memory_filler_impl<T, 1>
{
    static constexpr T value()
    {
        return (T)0xAB;
    }
};

template <typename T>
struct mp_memory_filler_impl<T, 2>
{
    static constexpr T value()
    {
        return (T)0xDEAD;
    }
};

template <typename T>
struct mp_memory_filler_impl<T, 3>
{
    static constexpr T value()
    {
        return (T)0xC0FFEE;
    }
};

template <typename T>
struct mp_memory_filler_impl<T, 4>
{
    static constexpr T value()
    {
        return (T)0xDEADBEEF;
    }
};

template <typename T>
struct mp_memory_filler_impl<T, 5>
{
    static constexpr T value()
    {
        return (T)0xC0FEEBEEF;
    }
};

template <typename T>
struct mp_memory_filler_impl<T, 6>
{
    static constexpr T value()
    {
        return (T)0xC0FEEC0FEE;
    }
};

template <typename T>
struct mp_memory_filler_impl<T, 7>
{
    static constexpr T value()
    {
        return (T)0xDEADBEEFC0FEE;
    }
};

template <typename T>
struct mp_memory_filler_impl<T, 8>
{
    static constexpr T value()
    {
        return (T)0xDEADBEEFDEADBEEF;
    }
};

template <typename List,
          bool... Statements>
struct mp_choose_more_impl;

template <typename FirstT,
          typename... RestT,
          template<typename...> typename List,
          bool... RestS>
struct mp_choose_more_impl<List<FirstT, RestT...>, true, RestS...>
{
    using type = FirstT;
};

template <typename FirstT,
          typename... RestT,
          template<typename...> typename List,
          bool... RestS>
struct mp_choose_more_impl<List<FirstT, RestT...>, false, RestS...>
{
    using type = mp_choose_more_impl<List<RestT...>, RestS...>;
};

template <typename List,
          typename OpT,
          u64... Numbers>
struct mp_choose_best_impl;

template <template <typename...> typename List,
          typename FirstT,
          typename SecondT,
          typename... RestT,
          typename OpT,
          u64 FirstN,
          u64 SecondN,
          u64... RestN>
struct mp_choose_best_impl<List<FirstT, SecondT, RestT...>, OpT, FirstN, SecondN, RestN...>
{
    static constexpr OpT _op{};

    using type = typename mp_choose_impl<
        _op(FirstN, SecondN),
        mp_choose_best_impl<List<FirstT, RestT...>, OpT, FirstN, RestN...>,
        mp_choose_best_impl<List<SecondT, RestT...>, OpT, SecondN, RestN...>
    >::type::type;
};

template <template <typename...> typename List,
          typename FirstT,
          typename OpT,
          u64 FirstN>
struct mp_choose_best_impl<List<FirstT>, OpT, FirstN>
{
    using type = FirstT;
};

/**
 * Take the template parameter list from the first argument
 * and return a type of the second argument with that parameter list.
 * A<Ts...> -> B<Ts...>
 * @param A What to rename.
 * @param B Result template class.
 */
template <typename A,
          template <typename...> typename B>
using mp_rename_t = typename mp_rename_impl<A, B>::type;

template <typename A,
          template <typename...> typename B>
using mp_rename = mp_rename_impl<A, B>;

/**
 * Test if the second parameter is specialization of template given
 * as the first parameter.
 * @param Base Check if this is the template class.
 * @param Spec The specialization to check.
 */
template <template <typename...> typename Base,
          typename Spec>
constexpr bool mp_is_specialization_v =
    mp_are_specializations_impl<Base, Spec>::value;

template <template <typename...> typename Base,
          typename Spec>
using mp_is_specialization =
    mp_are_specializations_impl<Base, Spec>;

/**
 * Test if all of the types in the parameter pack are specializations
 * of the template class given as the first parameter.
 * @param Base Check if this is the template class.
 * @param Specs Specializations to check.
 */
template <template <typename...> typename Base,
          typename... Specs>
constexpr bool mp_are_specializations_v =
    mp_are_specializations_impl<Base, Specs...>::value;

template <template <typename...> typename Base,
          typename... Specs>
using mp_are_specializations =
    mp_are_specializations_impl<Base, Specs...>;

/**
 * Call given function on parameter pack.
 * @param Fun The function.
 * @param ArgHolder Parameter pack.
 * @param Arguments Additional arguments
 */
template <template <typename...> typename Fun,
          typename ArgHolder,
          typename... Arguments>
using mp_call_on_arguments = mp_call_on_arguments_impl
                                <Fun, ArgHolder, Arguments...>;

/**
 * Check if given type list contains given element.
 * @param Element Element to look for.
 * @param List List to search through.
 */
template <typename Element,
          typename List>
using mp_contains = mp_contains_impl<Element, List>;

template <typename Element,
          typename List>
constexpr bool mp_contains_v = mp_contains_impl<Element, List>::value;

/**
 * Get length of given type list,
 * @param List Get the length of this list.
 */
template <typename List>
//using mp_size = mp_size_impl<List>;
using mp_size = mp_rename_t<List, mp_length_impl>;

template <typename List>
//constexpr u64 mp_size_v = mp_size_impl<List>::value;
constexpr u64 mp_size_v = mp_rename_t<List, mp_length_impl>::value;

/**
 * Get index of given type in given list.
 * @param Element What to search for.
 * @param List List to search through.
 */
template <typename Element,
          typename List>
using mp_index_of = mp_index_of_impl<Element, List>;

template <typename Element,
          typename List>
constexpr u64 mp_index_of_v = mp_index_of_impl<Element, List>::value;

/**
 * Fails to produce correct code, when the condition is false.
 * @param Cond The condition.
 * @param Type Type to return.
 */
template <bool Cond,
          typename Type>
using mp_fail_on_false_t = typename mp_fail_on_false_impl<Cond, Type>::type;

/**
 * Meta-function binder.
 * @param Fun The meta-function.
 * @param Params List of parameters to bind.
 */
template <template <typename...> typename Fun,
          typename... Params>
using mp_bind = mp_bind_impl<Fun, Params...>;

template <template <typename...> typename Fun,
          typename... Params>
using mp_bind_t = typename mp_bind_impl<Fun, Params...>::type;

/**
 * Transform given lists using a meta-function.
 * @param Fun Function to call on each element.
 * @param Lists Lists to apply the transformation on.
 */
template <template <typename...> typename Fun,
          typename... Lists>
using mp_transform = mp_transform_impl<Fun, Lists...>;

template <template <typename...> typename Fun,
          typename... Lists>
using mp_transform_t = typename mp_transform_impl<Fun, Lists...>::type;

/**
 * Accumulation meta-function. Take each element of the list
 * get its ::value and use given operator to "add" it to the
 * sum.
 * @param Op Operator to accumulate with.
 * @param List List of elements to accumulate.
 */
template <typename Op,
          typename List>
using mp_accumulate = mp_accumulate_impl<Op, List>;

template <typename Op,
          typename List>
static constexpr auto mp_accumulate_v = mp_accumulate_impl<Op, List>::value;

/**
 * Get Nth type from given type list.
 * @param N Index of the requested element, starts at 1!
 * @param List List containing the elements.
 */
template <u64 N,
          typename List>
using mp_get_nth = mp_get_nth_impl<N, List>;

template <u64 N,
          typename List>
using mp_get_nth_t = typename mp_get_nth_impl<N, List>::type;

/**
 * Removes const from given types.
 * @param T Type to remove const from.
 */
template <typename T>
using mp_remove_const = mp_remove_const_impl<T>;

template <typename T>
using mp_remove_const_t = typename mp_remove_const_impl<T>::type;

/**
 * Choose based on boolean flag.
 * @param flag Based on this flag the type is chosen.
 * @param TrueOut Output type, when the flag is true.
 * @param FalseOut Output type when the flag is false.
 */
template <bool flag,
          typename TrueOut,
          typename FalseOut>
using mp_choose = mp_choose_impl<flag, TrueOut, FalseOut>;

template <bool flag,
          typename TrueOut,
          typename FalseOut>
using mp_choose_t = typename mp_choose_impl<flag, TrueOut, FalseOut>::type;

/**
 * Check, if the two types are the same.
 * @param FirstT The first type.
 * @param SecondT The seconds type.
 */
template <typename FirstT,
          typename SecondT>
using mp_is_same = mp_is_same_impl<FirstT, SecondT>;

template <typename FirstT,
          typename SecondT>
static constexpr bool mp_is_same_v = mp_is_same_impl<FirstT, SecondT>::value;

/**
 * Easily recognizable memory patterns
 * @param T Generate pattern for this type
 */
template <typename T>
using mp_memory_filler = mp_memory_filler_impl<T>;

template <typename T>
static constexpr T mp_memory_filler_v = mp_memory_filler_impl<T>::value();

/**
 * Choose type corresponding with first true statement.
 * @param Types List of types, same size as the Statements pack.
 * @param Statements Pack of booleans.
 */
template <typename Types,
          bool... Statements>
using mp_choose_more = mp_choose_more_impl<Types, Statements...>;

template <typename Types,
          bool... Statements>
using mp_choose_more_t = typename mp_choose_more_impl<Types, Statements...>::type;

/**
 * Choose type corresponding with the number, that is (OP).
 * @param Types List of types, same size as the Numbers pack.
 * @param OpT Operator type used for choosing the best number.
 * @param Numbers Pack of numbers.
 */
template <typename Types,
          typename OpT,
          u64... Numbers>
using mp_choose_best = mp_choose_best_impl<Types, OpT, Numbers...>;

template <typename Types,
          typename OpT,
          u64... Numbers>
using mp_choose_best_t = typename mp_choose_best_impl<Types, OpT, Numbers...>::type;

#endif //UTIL_META_H
