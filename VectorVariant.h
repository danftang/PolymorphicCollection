//
// Created by daniel on 25/09/23.
//

#ifndef POLYMORPHICCOLLECTION_VECTORVARIANT_H
#define POLYMORPHICCOLLECTION_VECTORVARIANT_H

#include <variant>
#include <vector>

template<class...VARIANTTYPES>
class VectorVariant : protected std::variant<std::vector<VARIANTTYPES>...> {
public:
    using element_variant = std::variant<std::reference_wrapper<VARIANTTYPES>...>;
    using const_element_variant = std::variant<std::reference_wrapper<VARIANTTYPES>...>;
    using vector_variant_type = std::variant<std::vector<VARIANTTYPES>...>;


    template<class T> VectorVariant(std::in_place_type_t<T> /*type*/) :
    vector_variant_type(std::in_place_type_t<std::vector<T>>{}) { }


    /** Some helper functions for doing stuff with this vector without resolving its type.
     *
     * N.B. these will create a v-table for each variant, but it will be shared across all instances
     * so if we have instances for each variant type, then all entries will be used and memory usage will
     * be the same as if we had vectors with virtual base classes for each type.
     * Also, since the variant index never changes for each instance, the visit will be optimized out.
     */

    inline element_variant operator[](size_t index) {
        return std::visit([index]<class T>(std::vector<T> &vector) { return element_variant(vector[index]); }, asVariant());
    }

    inline element_variant operator[](size_t index) const {
        return std::visit([index]<class T>(const std::vector<T> &vector) { return const_element_variant(vector[index]); }, asVariant());
    }

    inline element_variant at(size_t index) {
        return std::visit([index]<class T>(std::vector<T> &vector) { return element_variant(vector.at(index)); }, asVariant());
    }

    inline const_element_variant at(size_t index) const {
        return std::visit([index]<class T>(const std::vector<T> &vector) { return const_element_variant(vector.at(index)); }, asVariant());
    }

    inline void reserve(size_t size) {
        std::visit([size]<class T>(std::vector<T> &vector) { vector.reserve(size); }, asVariant());
    }

    inline size_t size() const {
        return std::visit([]<class T>(const std::vector<T> &vector) { return vector.size(); }, asVariant());
    }

    inline size_t capacity() const {
        return std::visit([]<class T>(const std::vector<T> &vector) { return vector.capacity(); }, asVariant());
    }

    template<class FUNCTION>
    inline void for_each(FUNCTION &&func) {
        std::visit([f = std::forward<FUNCTION>(func)]<class T>(std::vector<T> &vector) {
            return std::for_each(vector.begin(), vector.end(), f);
        }, asVariant());
    }

    template<class FUNCTION>
    inline void for_each(FUNCTION &&func) const {
        std::visit([f = std::forward<FUNCTION>(func)]<class T>(const std::vector<T> &vector) {
            return std::for_each(vector.begin(), vector.end(), f);
        }, asVariant());
    }

    /** Only use this when calling std::visit, to get over the bug in libstdc++ that disallows
     * the use of classes derived from std::variant.
     * If you really want you can use this to modify the stored type of the Partition, but expect
     * insert() operators of ArrayOfPartitions to stop working if you do.
     *
     * @return the variant that this class inherits from (and otherwise hides).
     */
    inline vector_variant_type &asVariant() {
        return *this;
    }

    inline const vector_variant_type &asVariant() const {
        return *this;
    }
};



#endif //POLYMORPHICCOLLECTION_VECTORVARIANT_H
