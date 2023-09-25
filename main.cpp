#include <iostream>
#include <string>
#include <random>
#include "PolymorphicCollection.h"

using namespace std::string_literals;

int main() {
    // Make a collection that can contain ints, doubles and strings.
    // Internally, this is stored as a tuple of vectors.
    PolymorphicCollection<int,double,std::string> myCollection;

    // insert some objects
    myCollection.insert(1234);
    myCollection.insert(5.678);
    myCollection.insert("hello"s);

    // make a uniform integer distribution for choosing indices
    std::uniform_int_distribution<size_t> randomGenerator(0, 2);
    std::default_random_engine gen;

    for(int i=0; i<20; ++i) {
        // choose an item at random. Items are identified by
        // their type-partition and their index within the partition.
        ElementID randomItem {randomGenerator(gen),0};

        // Because we don't know the type of the chosen item at compile time,
        // we make a function that can deal with the item no matter
        // what type it turns out to be at runtime, and send it to the
        // visit(...) method.
        myCollection.visit(
                [](auto &item) {
                    std::cout << item << std::endl;
                },
                randomItem);

        // or we can get a std::variant of std::reference_wrappers out of the collection
        std::variant elementVariant = myCollection.get(randomItem);
        if(elementVariant.index() == 2) {
            std::string &str = std::get<2>(elementVariant);
            std::cout << "Found string " << str << std::endl;
        }
    }

    return 0;
}
