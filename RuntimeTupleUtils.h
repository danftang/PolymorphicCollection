//
// Created by daniel on 27/09/23.
//

#ifndef POLYMORPHICCOLLECTION_RUNTIMETUPLEUTILS_H
#define POLYMORPHICCOLLECTION_RUNTIMETUPLEUTILS_H

#include <cstdlib>
#include <tuple>
#include <ranges>
#include <variant>
#include <cassert>
#include "TypeUtils.h"

namespace deselby {
    namespace vtable {
        template<class TUPLE, class FUNCTION, size_t...Indices>
        auto do_indexed_apply(TUPLE &tuple, FUNCTION &&func) {
            return func(std::get<Indices>(tuple)...);
        }

        // seems pretty memory hungry, but this is effectively the way std::visit does it.
        // Has the advantage that a single lookup dispatches the function.
        template<class TUPLE, class FUNCTION, size_t Arity, size_t... vtableIndices>
        static consteval auto make_tuple_vtable(std::index_sequence<vtableIndices...>) {
            return std::array{
                    []<size_t...elementIndices>(std::index_sequence<elementIndices...>) {
                        return &do_indexed_apply<TUPLE, FUNCTION, elementIndices...>;
                    }(base_n_expansion_t<vtableIndices, Arity, std::tuple_size_v<TUPLE>>())...
            };
        }

        template<IsClassTemplateOf<std::tuple> TUPLE, class FUNCTION, size_t Arity>
        static constexpr auto tuple_v_table =
                make_tuple_vtable<TUPLE, FUNCTION, Arity>(
                        std::make_index_sequence<powi(std::tuple_size_v<TUPLE>, Arity)>());
    }


/** This is the tuple version of std::visit. The supplied function will be called with the
 * elements of the tuple indicated by the supplied indices.
 *
 * The function should accept the same number of arguments as the number of supplied indices,
 * and should be able to handle any combination of types from the tuple. The return type of the
 * function must be fixed for all argument types.
 *
 * @param tuple     The tuple from which to generate the arguments to the function
 * @param func      The function to call with the arguments identified by the tuple and indices
 * @param indices   The elements in the tuple that should be sent as arguments to the function
 * @return          The return value of the function invoked with the indicated arguments
 */
    template<IsClassTemplateOf<std::tuple> TUPLE, class FUNCTION, std::integral...INDICES>
    inline auto visit_tuple(TUPLE &tuple, FUNCTION &&func, INDICES...indices) {
        size_t vtableIndex = 0;
        ((vtableIndex = vtableIndex * std::tuple_size_v<TUPLE> + indices), ...);
        assert(vtableIndex < (vtable::tuple_v_table<TUPLE, FUNCTION, sizeof...(INDICES)>).size());
        return (vtable::tuple_v_table<TUPLE, FUNCTION, sizeof...(INDICES)>[vtableIndex])(tuple,
                                                                                         std::forward<FUNCTION>(func));
    }

/** iterate over a tuple */
    template<class FUNCTION, class...TYPES>
    void for_each(std::tuple<TYPES...> &tuple, FUNCTION &&func) {
        (func(std::get<TYPES>(tuple)), ...);
    }


/** make a variant whose value is the index'th element of the given tuple */
    template<class TUPLE>
    variant_type_t<TUPLE> make_variant(TUPLE &tuple, size_t index) {
        return visit_tuple(
                tuple,
                [](auto &item) {
                    return variant_type_t<TUPLE>(item);
                }, index);
    }


/** make a variant whose value is a reference_wrapper to the index'th element of the given tuple */
    template<class TUPLE>
    reference_variant_type_t<TUPLE> make_reference_variant(TUPLE &tuple, size_t index) {
        return visit_tuple(
                tuple,
                [](auto &item) {
                    return reference_variant_type_t<TUPLE>(item);
                },
                index);
    }


///////////////////////////////////////////////////
// convenience functions for tuples of ranges
// (also, reduces vtable size if you use standard functions)
///////////////////////////////////////////////////

/** Iterates over all elements of a tuple of ranges */
    template<class FUNCTION, std::ranges::range...TYPES>
    void for_each_element(std::tuple<TYPES...> &tuple, FUNCTION &&func) {
        (std::for_each(std::get<TYPES>(tuple).begin(), std::get<TYPES>(tuple).end(), func), ...);
    }


/** returns the size of an element in a tuple of sized_ranges */
    template<std::ranges::sized_range...TYPES>
    size_t size(const std::tuple<TYPES...> &tuple, size_t index) {
        return visit_tuple(tuple, [](auto &item) { return item.size(); }, index);
    }

///////// Even more functionality for a tuple of vectors //////////////

/** Pushes an item to the back of the corresponding vector in a tuple of vectors whose value_types are all unique */
    template<class T, class...ELEMENTTYPES>
    requires AllDifferentAndNotEmpty<ELEMENTTYPES...> && IsIn<std::remove_reference_t<T>, ELEMENTTYPES...>
    void push_back(std::tuple<std::vector<ELEMENTTYPES>...> &tuple, T &&item) {
        std::get<std::vector<std::remove_reference_t<T>>>(tuple).push_back(std::forward<T>(item));
    }
}
#endif //POLYMORPHICCOLLECTION_RUNTIMETUPLEUTILS_H
