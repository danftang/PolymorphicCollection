//
// Created by daniel on 23/09/23.
//

#ifndef UNTITLED_POLYMORPHICCOLLECTION_H
#define UNTITLED_POLYMORPHICCOLLECTION_H

#include <tuple>
#include <vector>
#include <variant>
#include <functional>

template<size_t suffix, size_t...prefix>
static std::index_sequence<prefix..., suffix> add_suffix(std::index_sequence<prefix...>) { return {}; }

template<size_t N, size_t digits, size_t base>
struct base_n_expansion : decltype(add_suffix<N%base>(base_n_expansion<N/base,digits-1,base>())) { };

template<size_t N, size_t base>
struct base_n_expansion<N,1,base> : std::index_sequence<N%base> { };

template<class T, class... OTHERS>
concept IsIn = (std::same_as<std::remove_cvref_t<T>,std::remove_cvref_t<OTHERS>> || ...);

template<class T, class...OTHERS> requires(!IsIn<T,OTHERS...>)
struct AllDifferentHelper: AllDifferentHelper<OTHERS...> {};

template<class T>
struct AllDifferentHelper<T> {};

template<class... CLASSES>
concept AllDifferentAndNotEmpty = requires() { AllDifferentHelper<CLASSES...>(); };


struct ElementID {
    size_t partition;
    size_t index;
};

/** Fill in for the lack of constexpr power function
 * Uses the identities:
 *   x^(2y)   = (x^y)^2
 *   x^(2y+1) = x * (x^y)^2
 *   the recursion ends with
 *   x^2 = x * x
 *   x^0 = 1
 */
template<std::integral T>
consteval T powi(T base, uint index) {
    if(index == 2) return base*base;
    if(index == 0) return 1;
    return (index&1) ? powi(powi(base,index/2),2) * base : powi(powi(base,index/2),2);
}

/** A PolymorphicCollection is a collection of objects whose type can be any one from a compile-time list.
 * The collection is split into partitions, one partition for each type. When an object is inserted into
 * the collection, it is inserted into the corresponding partition.
 *
 * Retreival of items from the collection can be split into two distinct modes:
 * 1) Compile-time typing
 *      In this case, the type of the required object (or it's partition index) is known at compile time,
 *      so we can retrieve either a vector of all entries of a given type using get<type or partitionID>()
 *      or a particular object using get<type or partitionID>(index)
 * 2) Runtime typing
 *      In this case, the type of the required object is only known at runtime, so any retrieval will result
 *      in a std::variant of reference wrappers, one variant for each type in the container.
 *      get(partitionID) will return a variant containing a whole partition, while
 *      get(partitionID,index) will return a variant of references to element types,
 *      instantiated with a reference to a single element.
 *      These variants can be applied to functions that take actual values using std::visit.
 *
 * @tparam CONTAINEDTYPES
 */
template<class...CONTAINEDTYPES> requires AllDifferentAndNotEmpty<CONTAINEDTYPES...>
class PolymorphicCollection {
public:
    using self_type = PolymorphicCollection<CONTAINEDTYPES...>;
    template<class T>  using partition_type = std::vector<T>;
    template<size_t N> using element_type   = std::tuple_element_t<N,std::tuple<CONTAINEDTYPES...>>;
    using partition_variant_type = std::variant<std::reference_wrapper<partition_type<CONTAINEDTYPES>>...>;
    using const_partition_variant_type = std::variant<std::reference_wrapper<const partition_type<CONTAINEDTYPES>>...>;
    using element_variant_type = std::variant<std::reference_wrapper<CONTAINEDTYPES>...>;
    using const_element_variant_type = std::variant<std::reference_wrapper<const CONTAINEDTYPES>...>;

    std::tuple<partition_type<CONTAINEDTYPES>...> dataTuple;

public:

    // ------ constructors -------------
    PolymorphicCollection(std::tuple<partition_type<CONTAINEDTYPES>...> init):
        dataTuple(std::move(init)) { };
    PolymorphicCollection() : PolymorphicCollection(std::tuple<partition_type<CONTAINEDTYPES>...>{}) {};

    // ------ compile-time typing -------
    template<class T>
    inline partition_type<T> &get() { return std::get<std::vector<T>>(dataTuple); }
    template<class T>
    inline const partition_type<T> &get() const { return std::get<std::vector<T>>(dataTuple); }

    template<size_t N>
    inline partition_type<element_type<N>> &get() { return std::get<N>(dataTuple); }
    template<size_t N>
    inline const partition_type<element_type<N>> &get() const { return std::get<N>(dataTuple); }

    template<IsIn<CONTAINEDTYPES...> T>
    inline void insert(T &&element) { get<std::remove_cvref_t<T>>().push_back(std::forward<T>(element)); }

    template<class T>
    T &get(size_t index) { return get<T>().at(index); }
    template<class T>
    const T &get(size_t index) const { return get<T>().at(index); }

    template<size_t N>
    element_type<N> &get(size_t index) { return get<N>().at(index); }
    template<size_t N>
    const element_type<N> &get(size_t index) const { return get<N>().at(index); }

    // --------- runtime typing --------------

    /** Executes a given function with just one vtable lookup.
     *
     * @tparam FUNCTION
     * @param func
     * @param partitionID
     * @param index
     */
    template<class FUNCTION>
    void visit(FUNCTION &&func, ElementID elementId) {
        using vtable_entry = void(self_type::*)(FUNCTION &&, size_t);
        static constexpr std::array<vtable_entry, sizeof...(CONTAINEDTYPES)> vtable{
            &self_type::visitExecutor<FUNCTION,CONTAINEDTYPES>...
        };
        (this->*vtable[elementId.partition])(std::forward<FUNCTION>(func), elementId.index);
    }


    template<class FUNCTION, std::convertible_to<ElementID>... ELEMENTS>
    void visit(FUNCTION &&func, const ELEMENTS &... elementIDs) {
        static constexpr auto vtable = makeMultiargVTable<FUNCTION,ELEMENTS...>(std::make_index_sequence<powi(sizeof...(CONTAINEDTYPES),sizeof...(ELEMENTS))>());
        size_t vtableIndex = 0;
        ([&vtableIndex](ElementID element) {
            vtableIndex *= sizeof...(CONTAINEDTYPES);
            vtableIndex += element.partition;
        }(elementIDs),...);
        (this->*vtable[vtableIndex])(std::forward<FUNCTION>(func), elementIDs...);
    }


    template<class FUNCTION>
    void for_each(FUNCTION &&function) {
        (std::for_each(get<CONTAINEDTYPES>().begin(), get<CONTAINEDTYPES>().end(),function),...);
    }

    inline partition_variant_type get(size_t partitionID) {
        return (this->*partitionGetters[partitionID])();
    }

    inline const_partition_variant_type get(size_t partitionID) const {
        return (this->*constPartitionGetters[partitionID])();
    }

    element_variant_type get(size_t partitionID, size_t index) {
        return (this->*elementGetters[partitionID])(index);
    }

    const_element_variant_type get(size_t partitionID, size_t index) const {
        return (this->*constElementGetters[partitionID])(index);
    }

    element_variant_type get(const ElementID &element) {
        return (this->*elementGetters[element.partition])(element.index);
    }

    const_element_variant_type get(const ElementID &element) const {
        return (this->*constElementGetters[element.partition])(element.index);
    }

//protected:
    template<class FUNCTION, class T>
    void visitExecutor(FUNCTION &&func, size_t index) {
        func(get<T>(index));
    }

    template<class FUNCTION, size_t... partitionIDs, class...ELEMENTS>
    inline void multiargVisitExecutorImpl(FUNCTION &&func, std::index_sequence<partitionIDs...> seq, const ELEMENTS &...elements) {
        func(get<partitionIDs>(elements.index)...);
    }

    template<class FUNCTION, size_t compoundPartitionID, class... ELEMENTS>
    void multiargVisitExecutor(FUNCTION &&func, const ELEMENTS &... elements) {
        multiargVisitExecutorImpl(std::forward<FUNCTION>(func), base_n_expansion<compoundPartitionID,sizeof...(ELEMENTS),sizeof...(CONTAINEDTYPES)>() , elements...);
    }

    template<class FUNCTION, class...ELEMENTS, size_t... indices>
    static consteval auto makeMultiargVTable(std::index_sequence<indices...>) {
        using vtable_entry = void(self_type::*)(FUNCTION &&, const ELEMENTS &...);
        return std::array<vtable_entry, sizeof...(indices)>{
                &self_type::multiargVisitExecutor<FUNCTION,indices,ELEMENTS...>...
        };

    }

    template<class T>
    partition_variant_type getPartitionVariant() {
        return std::get<partition_type<T>>(dataTuple);
    }

    template<class T>
    const_partition_variant_type getConstPartitionVariant() const {
        return std::get<partition_type<T>>(dataTuple);
    }

    template<class T>
    element_variant_type getElementVariant(size_t index) {
        return std::get<partition_type<T>>(dataTuple)[index];
    }

    template<class T>
    const_element_variant_type getConstElementVariant(size_t index) const {
        return std::get<partition_type<T>>(dataTuple)[index];
    }

    static constexpr std::array<
            partition_variant_type(PolymorphicCollection<CONTAINEDTYPES...>::*)(),
            sizeof...(CONTAINEDTYPES)>
            partitionGetters{&PolymorphicCollection<CONTAINEDTYPES...>::getPartitionVariant<CONTAINEDTYPES>...};

    static constexpr std::array<
        const_partition_variant_type(PolymorphicCollection<CONTAINEDTYPES...>::*)() const,
        sizeof...(CONTAINEDTYPES)>
            constPartitionGetters{&PolymorphicCollection<CONTAINEDTYPES...>::getConstPartitionVariant<CONTAINEDTYPES>...};

    static constexpr std::array<
            element_variant_type(PolymorphicCollection<CONTAINEDTYPES...>::*)(size_t),
            sizeof...(CONTAINEDTYPES)>
            elementGetters{&PolymorphicCollection<CONTAINEDTYPES...>::getElementVariant<CONTAINEDTYPES>...};

    static constexpr std::array<
            const_element_variant_type(PolymorphicCollection<CONTAINEDTYPES...>::*)(size_t) const,
            sizeof...(CONTAINEDTYPES)>
            constElementGetters{&PolymorphicCollection<CONTAINEDTYPES...>::getConstElementVariant<CONTAINEDTYPES>...};
};

#endif //UNTITLED_POLYMORPHICCOLLECTION_H
