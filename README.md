# Runtime indexing of std::tuple (and its use for polymorphic collections)

If you want to create a C++ collection that contains different types, the obvious way is to use a vector of unique_ptrs to some base class. However, this has the following disadvantages:
  * the objects are stored at random memory locations, making iteration potentially slower
  * you can only store types that inherit from the base class
  * each call to a virtual function requires a vtable lookup
  * For each element, in addition to the data in the element, we also need to store a unique_ptr and (because the
    classes implement virtual functions) the object will store a pointer to its vtable, identifying its dynamic type.

If we know which types we will allow in the container at compile time, a more efficient way is to use a tuple of vectors with one vector for each type. However, tuples have the big disadvantage that they don't allow runtime indexing of the elements. So we can't, for example, choose a random element and do something with it.

However, with the addition of std::variant and std::visit to the standard library it becomes possible, in principle at least, to have runtime indexing of the elements of a tuple by generating a std::variant to refer to a tuple element at runtime.

The utilities here allow us runtime access to tuple elements using:

```
std::tuple<...> myTuple;
...
size_t index = ...some runtime calculation...

std::variant tupleElement            = make_variant(myTuple, index);
std::variant referenceToTupleElement = make_variant_reference(myTuple, index);

std::visit([](auto &element) { ...do something with element... }, tupleElement);
```

`tupleElement` contains an element of `myTuple` and `referenceToTupleElement`  contains a `std::reference_wrapper<.>` to an element of `myTuple`. The elements can be accessed with `std::visit`.

However, given the tuple, the index itself can act as a variant, allowing us to do away with std::variants. For this we provide `visit_tuple` as a tupled-up version of `std:visit`, allowing us to write:
```
std::tuple<...> myTuple;
...
size_t index = ...some runtime calculation...

visit_tuple(myTuple, [](auto &element) { ...do something with element... }, index);
```
Runtime dispatch to the function is achieved in a single vtable lookup, which will be optimised out if it is deducible at compile-time.

## Polymorphic collections

We can make a fast collection of unrelated types (i.e. no shared base class) by making a tuple of containers and using these utilities to access it. When types are known at compile time, use standard tuple access (e.g. `std::get`), when types are only known at runtime, use `visit_tuple`. For example
```
std::tuple<std::array<double,10>, std::array<std::string,10>> myTuple;
...
size_t index = ...some runtime calculation...

visit_tuple(myTuple, [](auto &myArray) { std::cout << "5th element is " << myArray[5] << '\n'; }, index);
```

### C++ duck-typing

In this way we have achieved duck-typing in C++: `myTuple` above can contain any object that provides  an `operator []` that returns a type that provides `operator <<` to a `std::ostream`, without needing a shared base class and dispatch via virtual methods.

See `main.cpp` for more examples.

We also supply some convenience functions specifically for tuples of ranges and tuples of vectors. For example `for_all_elements` to iterate over all elements in the tuple, or `size` to get the size of a runtime calculated element.
