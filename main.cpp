#include <iostream>
#include <string>
#include <random>
//#include "TupleOfVectors.h"
#include "VectorVariant.h"
#include "ArrayOfPartitions.h"
#include "RuntimeTupleUtils.h"

using namespace std::string_literals;

void myFunc(int &) {}

int main() {
    // =======================================================
    // Runtime access to tuples
    // =======================================================

    // Make a tuple
    std::tuple<int,double,char> myTuple(1234,5.678,'a');

    // Generate some random indices to refer to elements of the tuple which aren't known at compile time
    std::uniform_int_distribution<size_t> randomGenerator(0, 2);
    std::default_random_engine gen;

    // use visit_tuple to do something with the elements at the given indices
    for(int i=0; i<10; ++i) {
        int index  = randomGenerator(gen);
        visit_tuple(myTuple, [index](auto &item) { std::cout << "item at index: " << index << " = " << item << std::endl; }, index);
    }
    int index  = randomGenerator(gen);
    int index2 = randomGenerator(gen);
    visit_tuple(myTuple, [](auto &item1, auto &item2) { std::cout << "binary function: " << item1 << " " << item2 << std::endl; }, index, index2);

    // ...or we can generate a std::variant using make_variant
    auto variant = make_variant(myTuple, index);
    std::visit([](auto &item) { std::cout << "item from variant: " << item << std::endl; }, variant);

    // ...if the tuple stores large objects, we can generate a std::variant of std::reference_wrappers to the element
    auto refVariant = make_reference_variant(myTuple, index);
    std::visit([](auto &item) { std::cout << "item from reference variant: " << item.get() << std::endl; }, refVariant);

    // =======================================================
    // Additional functionality for tuples of ranges
    // =======================================================

    std::tuple<std::vector<int>, std::array<double,4>> myTupleOfRanges;

    std::get<0>(myTupleOfRanges).push_back(1234);
    std::get<1>(myTupleOfRanges)[0] = 4.567;

    size_t index3 = std::uniform_int_distribution<size_t>(0, 1)(gen);

    std::cout << "size of container " << index3 << " = " << size(myTupleOfRanges,index3) << std::endl;

    // iterate over all elements in all vectors in the tuple.
    for_each_element(myTupleOfRanges, [](auto &item) { item += 1; });

    std::cout << "First element of vector is now " << std::get<0>(myTupleOfRanges)[0] << std::endl;

    // =======================================================
    // Additional functionality for tuples of vectors
    // =======================================================

    std::tuple<std::vector<int>, std::vector<double>> myTupleOfVectors;

    // push items into a tuple of vectors, the correct vector is identified by the type of the element.
    push_back(myTupleOfVectors,1234);   // inserts into std::vector<int>
    push_back(myTupleOfVectors,1.234);  // inserts into std::vector<double>

    for_each_element(myTupleOfVectors, [](auto &element) { std::cout << element << std::endl; });

    return 0;
}
