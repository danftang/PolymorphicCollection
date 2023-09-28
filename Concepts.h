//
// Created by daniel on 27/09/23.
//

#ifndef POLYMORPHICCOLLECTION_CONCEPTS_H
#define POLYMORPHICCOLLECTION_CONCEPTS_H

#include <cstdlib>
#include <utility>

template<class T, template<class...> class TEMPLATE>
struct is_class_template_of {
    static constexpr bool value = false;
};

template<template<class...> class TEMPLATEDCLASS, class...TEMPLATES>
struct is_class_template_of<TEMPLATEDCLASS<TEMPLATES...>,TEMPLATEDCLASS> {
    static constexpr bool value = true;
};

template<template<class...> class TEMPLATEDCLASS, class...TEMPLATES>
struct is_class_template_of<const TEMPLATEDCLASS<TEMPLATES...>,TEMPLATEDCLASS> {
    static constexpr bool value = true;
};

template<class T, template<class...> class TEMPLATE>
static constexpr bool is_template_of_v = is_class_template_of<T,TEMPLATE>::value;

template<class T, template<class...> class TEMPLATE>
concept IsClassTemplateOf = is_template_of_v<T,TEMPLATE>;


/** Expresses a number as a sequence of digits in a given base
 *
 * @tparam N        the number to express
 * @tparam digits   how many digits to expand
 * @tparam base     the base to use in expansion
 * @tparam suffix   an optional suffix which will be added to the end of the expansion
 */
template<size_t N, size_t digits, size_t base, size_t...suffix>
struct base_n_expansion : base_n_expansion<N/base, digits - 1, base, N%base, suffix...> { };

template<size_t N, size_t base, size_t... suffix>
struct base_n_expansion<N, 1, base, suffix...> {
    using type = typename std::index_sequence<N%base, suffix...>;
};

template<size_t N, size_t digits, size_t base>
using base_n_expansion_t = typename base_n_expansion<N,digits,base>::type;


template<class T, class... OTHERS>
concept IsIn = (std::same_as<std::remove_cvref_t<T>,std::remove_cvref_t<OTHERS>> || ...);

template<class T, class...OTHERS> requires(!IsIn<T,OTHERS...>)
struct AllDifferentHelper: AllDifferentHelper<OTHERS...> {};

template<class T>
struct AllDifferentHelper<T> {};

template<class... CLASSES>
concept AllDifferentAndNotEmpty = requires() { AllDifferentHelper<CLASSES...>(); };

template<std::integral T>
consteval T powi(T base, uint index) {
    if(index == 2) return base*base;
    if(index == 0) return 1;
    return (index&1) ? powi(powi(base,index/2),2) * base : powi(powi(base,index/2),2);
}

template<class T, T...indices>
std::ostream &operator <<(std::ostream &out, std::integer_sequence<T,indices...> /**/) {
    ((out << indices << " "),...);
    return out;
}

#endif //POLYMORPHICCOLLECTION_CONCEPTS_H
