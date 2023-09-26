#include <iostream>
#include <string>
#include <random>
#include "TupleOfVectors.h"
#include "Partition.h"
#include "ArrayOfPartitions.h"

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
            std::cout << "Found string " << maybeAStringVector->get()[0] << std::endl;
        }
    }

    return 0;
}
