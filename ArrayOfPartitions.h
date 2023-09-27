//
// Created by daniel on 26/09/23.
//

#ifndef POLYMORPHICCOLLECTION_ARRAYOFVARIANTS_H
#define POLYMORPHICCOLLECTION_ARRAYOFVARIANTS_H

#include <array>
#include <variant>
#include "VectorVariant.h"
#include "Concepts.h"

struct ElementIndex {
    size_t partition;
    size_t index;
};

template<class...ELEMENTTYPES> requires AllDifferentAndNotEmpty<ELEMENTTYPES...>
class ArrayOfPartitions : public std::array<VectorVariant<ELEMENTTYPES...>, sizeof...(ELEMENTTYPES)> {
public:

    ArrayOfPartitions() : std::array<VectorVariant<ELEMENTTYPES...>, sizeof...(ELEMENTTYPES)>{
            VectorVariant<ELEMENTTYPES...>(std::in_place_type_t<ELEMENTTYPES>{})...
    } {};

    template<size_t INDEX>
    inline auto &partition() {
        return std::get<INDEX>((*this)[INDEX].asVariant());
    }

    template<size_t INDEX>
    inline const auto &partition() const {
        return std::get<INDEX>((*this)[INDEX].asVariant());
    }

    template<class T>
    inline std::vector<T> &partition() { return partition<typeToIndex<T>()>(); }

    template<class T>
    inline const std::vector<T> &partition() const { return partition<typeToIndex<T>()>(); }

    VectorVariant<ELEMENTTYPES...> &operator [](size_t partitionID) {
        return std::array<VectorVariant<ELEMENTTYPES...>,sizeof...(ELEMENTTYPES)>::operator [](partitionID);
    }

    std::variant<std::reference_wrapper<ELEMENTTYPES>...> operator [](const ElementIndex &elementId) {
        return (*this)[elementId.partition][elementId.index];
    }

    template<class T>
    void insert(T &&item) {
        partition<std::remove_cvref_t<T>>().push_back(item);
    }


    template<class FUNCTION, class... ELEMENTS> requires (std::same_as<ELEMENTS,ElementIndex>,...)
    void visit(FUNCTION &&func, ELEMENTS...elements) {
        std::visit([&func]<class...T>(std::reference_wrapper<T>... refs) {
            func(refs.get()...);
        }, (*this)[elements]...);
    }

    template<class FUNCTION, std::integral... PARTITIONIDS>
    void visit(FUNCTION &&func, PARTITIONIDS...partitionIds) {
        std::visit(func, (*this)[partitionIds].asVariant()...);
    }


    /** Use this to iterate over all elements
     *
     * @param function  An opbject that can be invoked on a reference to any of the contained element types
     *                  (e.g. a lambda with auto type)
     */
    template<class FUNCTION>
    void for_each_element(FUNCTION &&function) {
        (std::for_each(partition<ELEMENTTYPES>().begin(), partition<ELEMENTTYPES>().end(), function),...);
    }

    template<class FUNCTION>
    void for_each_element(FUNCTION &&function) const {
        (std::for_each(partition<ELEMENTTYPES>().begin(), partition<ELEMENTTYPES>().end(), function),...);
    }

    /** Use this to iterate over all partitions
     *
     * @param function An opbject that can be invoked on a reference to a std::vector of any of the
     *                 contained types (e.g. a lambda with auto type).
     */
    template<class FUNCTION>
    void for_each_partition(FUNCTION &&function) {
        (function(partition<ELEMENTTYPES>()),...);
    }

    template<class FUNCTION>
    void for_each_partition(FUNCTION &&function) const {
        (function(partition<ELEMENTTYPES>()),...);
    }


    size_t elementCount() const {
        return (partition<ELEMENTTYPES>().size() + ...);
    }

    template<class T>
    static consteval size_t typeToIndex() {
        size_t index = 0;
        return ((std::same_as<T,ELEMENTTYPES> * index++) + ... );
    }

};


#endif //POLYMORPHICCOLLECTION_ARRAYOFVARIANTS_H
