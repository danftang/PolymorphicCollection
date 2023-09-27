#include <iostream>
#include <string>
#include <random>
//#include "TupleOfVectors.h"
#include "VectorVariant.h"
#include "ArrayOfPartitions.h"
#include "TupleHelpers.h"

using namespace std::string_literals;

int main() {
    // Make a collection that can contain ints, doubles and strings.
    //    PolymorphicCollection<int,double,std::string> myCollection;
    ArrayOfPartitions<int,double,std::string> myCollection;

    // insert some objects of different types
    myCollection.insert(1234);
    myCollection.insert(5.678);
    myCollection.insert("hello"s);

    // make a uniform integer distribution for choosing indices
    std::uniform_int_distribution<size_t> randomGenerator(0, 2);
    std::default_random_engine gen;

    // make a function to print a reference_wrapper to any type that has a stream operator
    auto myPrintFunction = []<class T>(std::reference_wrapper<T> item) {
        std::cout << item.get() << std::endl;
    };

    for(int i=0; i<20; ++i) {
        // choose a partition at random.
        size_t randomPartition = randomGenerator(gen);

        // Now get the first item in the chosen partition.
        auto elementVariant = myCollection[randomPartition][0];

        // Now print the element.
        // Because we don't know the type of the element at compile time,
        // (elementVariant is a std::variant of std::reference_wrappers)
        // we make a function that can deal with items no matter
        // what type it turns out to be at runtime, and send it to the
        // visit(...) method.
        std::visit(myPrintFunction, elementVariant);

        // or we can get a variant of the whole partition
        auto &partitionVariant = myCollection[randomPartition].asVariant();
        auto *maybeAStringVector = std::get_if<2>(&partitionVariant);
        if(maybeAStringVector != nullptr) {
            std::cout << "Found string " << (*maybeAStringVector)[0] << std::endl;
        }
    }

    // we can iterate over all partitions
    myCollection.for_each_partition([]<class T>(std::vector<T> &vec) {
        std::cout << "Partition of type " << typeid(T).name() << " has size " << vec.size() << std::endl;
    });

    // or we can iterate over all elements
    myCollection.for_each_element([](auto &item) {
        std::cout << "Element: " << item << std::endl;
    });

    myCollection.visit([](auto &item) { std::cout << "Single element access: " <<  item << std::endl; }, ElementIndex{0,0});


    // =======================================================
    // Runtime access to tuples
    // =======================================================

    std::tuple<int,double,char> myTuple(1234,5.678,'a');

    int index  = randomGenerator(gen);
    int index2 = randomGenerator(gen);

    visit_tuple([](auto &item) { std::cout << item << std::endl; }, myTuple, index);
    visit_tuple([](auto &item1, auto &item2) { std::cout << item1 << " " << item2 << std::endl; }, myTuple, index, index2);

    return 0;
}
