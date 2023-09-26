//
// Created by daniel on 25/09/23.
//

#ifndef POLYMORPHICCOLLECTION_PARTITION_H
#define POLYMORPHICCOLLECTION_PARTITION_H

#include <variant>
#include <vector>

template<class...VARIANTTYPES>
class Partition {
public:
    using element_variant = std::variant<std::reference_wrapper<VARIANTTYPES>...>;
    using const_element_variant = std::variant<std::reference_wrapper<VARIANTTYPES>...>;
    using vector_ref_variant_type = std::variant<std::reference_wrapper<std::vector<VARIANTTYPES>>...>;
    using const_vector_ref_variant_type = std::variant<std::reference_wrapper<const std::vector<VARIANTTYPES>>...>;


    template<class T> Partition(std::in_place_type_t<T> /*type*/) :
    variant(std::in_place_type_t<std::vector<T>>{}),
    vectorRefVariant(std::in_place_type_t<std::reference_wrapper<std::vector<T>>>{},std::get<std::vector<T>>(variant)),
    constVectorRefVariant(std::in_place_type_t<std::reference_wrapper<const std::vector<T>>>{},std::get<std::vector<T>>(variant))
    {
    }



    /** N.B. these will create a vtable for each variant, but it will be shared across all instances
     * so if we have instances for each variant, then all entries will be used and memory usage will
     * be the same as if we had vectors with virtual base classes for each variant.
     * Also, since the variant index never changes for each instance, the visit will be optimized out.
     */

    inline element_variant operator[](size_t index) {
        return std::visit([index](auto vector) { return element_variant(vector.get()[index]); }, asVariant());
    }

    inline element_variant operator[](size_t index) const {
        return std::visit([index](auto vector) { return const_element_variant(vector.get()[index]); }, asVariant());
    }

    inline element_variant at(size_t index) {
        return std::visit([index](auto vector) { return element_variant(vector.get().at(index)); }, asVariant());
    }

    inline const_element_variant at(size_t index) const {
        return std::visit([index](auto vector) { return const_element_variant(vector.get().at(index)); }, asVariant());
    }

    inline void reserve(size_t size) {
        std::visit([size](auto vector) { vector.get().reserve(size); }, asVariant());
    }

    inline size_t size() const {
        return std::visit([](auto vector) { return vector.get().size(); }, asVariant());
    }

    inline size_t capacity() const {
        return std::visit([](auto vector) { return vector.get().capacity(); }, asVariant());
    }

    template<class FUNCTION>
    inline void for_each(FUNCTION &&func) {
        std::visit([f = std::forward<FUNCTION>(func)](auto vector) { return std::for_each(vector.get().begin(), vector.get().end(), f); }, asVariant());
    }

    template<class FUNCTION>
    inline void for_each(FUNCTION &&func) const {
        std::visit([f = std::forward<FUNCTION>(func)](auto vector) { return std::for_each(vector.get().begin(), vector.get().end(), f); }, asVariant());
    }

    inline const vector_ref_variant_type &asVariant() {
        return vectorRefVariant;
    }

    inline const const_vector_ref_variant_type &asVariant() const {
        return constVectorRefVariant;
    }

protected:
    std::variant<std::vector<VARIANTTYPES>...>      variant; // just for storage really, we use the refs for access.
                                                             // This ensures the varaint's type is never modified.
    const vector_ref_variant_type                   vectorRefVariant;
    const const_vector_ref_variant_type             constVectorRefVariant;

};



#endif //POLYMORPHICCOLLECTION_PARTITION_H
