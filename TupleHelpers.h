//
// Created by daniel on 27/09/23.
//

#ifndef POLYMORPHICCOLLECTION_TUPLEHELPERS_H
#define POLYMORPHICCOLLECTION_TUPLEHELPERS_H

#include <cstdlib>
#include <tuple>
#include <ranges>
#include <variant>
#include "Concepts.h"

template<class... T>
std::ostream &operator <<(std::ostream &out, const std::tuple<T...> &tuple) {
    ((out << std::get<T>(tuple) << " "),...);
    return out;
}

///** returns a tuple of references to the elements at the given indices */
//template<class...ELEMENTS, size_t...Indices>
//auto multi_get(std::tuple<ELEMENTS...> tuple) {
//    return std::make_tuple(std::get<Indices>(tuple)...);
//}

/** returns a tuple of references to the elements at the given indices */
template<class FUNCTION, IsTemplateOf<std::tuple> TUPLE, size_t...Indices>
auto do_indexed_apply(TUPLE &tuple, FUNCTION &&func) {
    return func(std::get<Indices>(tuple)...);
}

template<class FUNCTION, class... TUPLEELEMENTS, size_t...Indices>
auto do_indexed_apply(FUNCTION &&func, TUPLEELEMENTS &&...elements) {
//    std::get<0>(elements...);
//    return func(std::get<Indices>(tuple)...);
}

template<class FUNCTION, class...ELEMENTS, size_t...Indices>
auto indexed_apply(std::tuple<ELEMENTS...> &tuple, std::index_sequence<Indices...> /*elements to apply*/, FUNCTION &&func) {
    return do_indexed_apply<FUNCTION,ELEMENTS...,Indices...>(tuple, std::forward<FUNCTION>(func));
}

template<class FUNCTION, size_t Arity, class...ELEMENTS, size_t... vtableIndices>
static consteval auto make_tuple_vtable(std::index_sequence<vtableIndices...>) {
    return std::array{
            []<size_t...elementIndices>(std::index_sequence<elementIndices...>) {
                return &do_indexed_apply<FUNCTION,std::tuple<ELEMENTS...>,elementIndices...>;
            }(base_n_expansion<vtableIndices,Arity,sizeof...(ELEMENTS)>())...
    };
}

template<class FUNCTION, size_t Arity, class...ELEMENTS>
static constexpr auto tuple_v_table =
        make_tuple_vtable<FUNCTION,Arity,ELEMENTS...>(std::make_index_sequence<powi(sizeof...(ELEMENTS),Arity)>());


template<class FUNCTION, class...ELEMENTS, std::integral...INDICES>
auto visit_tuple(FUNCTION &&func, std::tuple<ELEMENTS...> &tuple, INDICES...indices) {
    size_t vtableIndex = 0;
    ((vtableIndex = vtableIndex*sizeof...(ELEMENTS) + indices),...);
    return (tuple_v_table<FUNCTION,sizeof...(INDICES),ELEMENTS...>[vtableIndex])(tuple, std::forward<FUNCTION>(func));
}

//template<class FUNCTION, class...ELEMENTS, std::integral...INDICES>
//auto visit_tuple(FUNCTION &&func, const std::tuple<ELEMENTS...> &tuple, INDICES...indices) {
//    // TODO:...
//}

template<class FUNCTION, class...TYPES>
void for_each(std::tuple<TYPES...> &tuple, FUNCTION &&func) {
    (func(std::get<TYPES>(tuple)),...);
}

template<class FUNCTION, std::ranges::range...TYPES>
void for_each_element(std::tuple<TYPES...> &tuple, FUNCTION &&func) {
    (std::for_each(std::get<TYPES>(tuple).begin(),std::get<TYPES>(tuple).end(),func),...);
}

template<class...ELEMENTS> requires AllDifferentAndNotEmpty<ELEMENTS...>
std::variant<ELEMENTS...> get_variant(const std::tuple<ELEMENTS...> &tuple, size_t index) {
    return visit_tuple(
            []<class T>(const T &item) {
                return std::variant<ELEMENTS...>(item);
            },
            tuple, index);
}

template<class...ELEMENTS> requires AllDifferentAndNotEmpty<ELEMENTS...>
std::variant<std::reference_wrapper<ELEMENTS>...> get_reference_variant(std::tuple<ELEMENTS...> &tuple, size_t index) {
    return visit_tuple(
            [&tuple]<class T>(const T &item) {
                return std::variant<std::reference_wrapper<ELEMENTS>...>(std::get<T>(tuple));
            },
            tuple, index);
}

template<class...ELEMENTS> requires AllDifferentAndNotEmpty<ELEMENTS...>
std::variant<std::reference_wrapper<const ELEMENTS>...> get_reference_variant(const std::tuple<ELEMENTS...> &tuple, size_t index) {
    return visit_tuple(
            [&tuple]<class T>(const T &item) {
                return std::variant<std::reference_wrapper<const ELEMENTS>...>(std::get<T>(tuple));
            },
            tuple, index);
}



#endif //POLYMORPHICCOLLECTION_TUPLEHELPERS_H
