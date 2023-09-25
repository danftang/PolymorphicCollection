//
// Created by daniel on 25/09/23.
//

#ifndef POLYMORPHICCOLLECTION_POLYMORPHICVECTORREFERENCE_H
#define POLYMORPHICCOLLECTION_POLYMORPHICVECTORREFERENCE_H

#include <vector>
#include <variant>

template<class...VARIANTTYPES>
class VariantVectorReference {
public:
    virtual std::variant<VARIANTTYPES...> operator[](size_t index) = 0;
};

template<class REFTYPE, class... VARIANTTYPES>
class VectorReference : public VariantVectorReference<VARIANTTYPES...> {
public:
    std::vector<REFTYPE> &vector;

    std::variant<VARIANTTYPES...> operator[](size_t index) = 0;

};



#endif //POLYMORPHICCOLLECTION_POLYMORPHICVECTORREFERENCE_H
