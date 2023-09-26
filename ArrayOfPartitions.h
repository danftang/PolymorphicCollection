//
// Created by daniel on 26/09/23.
//

#ifndef POLYMORPHICCOLLECTION_ARRAYOFVARIANTS_H
#define POLYMORPHICCOLLECTION_ARRAYOFVARIANTS_H

#include <array>
#include <variant>
#include "Partition.h"

template<class...PARTITIONTYPES> requires AllDifferentAndNotEmpty<PARTITIONTYPES...>
class ArrayOfPartitions : public std::array<Partition<PARTITIONTYPES...>, sizeof...(PARTITIONTYPES)> {
public:
    ArrayOfPartitions() : std::array<Partition<PARTITIONTYPES...>, sizeof...(PARTITIONTYPES)>{
        Partition<PARTITIONTYPES...>(std::in_place_type_t<PARTITIONTYPES>{})...
    } {};

    template<size_t INDEX>
    inline auto &partition() {
        return std::get<INDEX>((*this)[INDEX].asVariant()).get();
    }

    template<size_t INDEX>
    inline const auto &partition() const {
        return std::get<INDEX>((*this)[INDEX].asVariant()).get();
    }

    template<class T>
    inline std::vector<T> &partition() { return partition<typeToIndex<T>()>(); }

    template<class T>
    inline const std::vector<T> &partition() const { return partition<typeToIndex<T>()>(); }


    template<class T>
    void insert(T &&item) {
        partition<std::remove_cvref_t<T>>().push_back(item);
    }

    template<class FUNCTION>
    void for_each(FUNCTION &&function) {
        (std::for_each(partition<PARTITIONTYPES>().begin(), partition<PARTITIONTYPES>().end(),function),...);
    }

    template<class FUNCTION>
    void for_each(FUNCTION &&function) const {
        (std::for_each(partition<PARTITIONTYPES>().begin(), partition<PARTITIONTYPES>().end(),function),...);
    }

    template<class T>
    static consteval size_t typeToIndex() {
        size_t index = 0;
        return ((std::same_as<T,PARTITIONTYPES>*index++) + ... );
    }

    size_t size() const {
        return (partition<PARTITIONTYPES>().size() + ...);
    }
};


#endif //POLYMORPHICCOLLECTION_ARRAYOFVARIANTS_H
