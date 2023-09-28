# Runtime indexing of std::tuple (and its use for polymorphic collections)

If you want to create a C++ collection that contains different types, the obvious way is to use a vector of unique_ptrs
to some base class. However, this has the following disadvantages:
  * the objects are stored at random memory locations, making iteration potentially slower
  * you can only store types that inherit from the base class
  * each call to a virtual function requires a vtable lookup
  * For each element, in addition to the data in the element, we also need to store a unique_ptr and (because the
    classes implement virtual functions) the object will store a pointer to its vtable, identifying its dynamic type.

If we know which types we will allow in the container at compile time, a more efficient way is to use a tuple of vectors 
with one vector for each type. However, tuples have the big disadvantage that they don't allow runtime
indexing of the elements. So we can't, for example, choose a random element and do something with it.

However, with the addition of std::variant and std::visit to the standard library 
it becomes possible, in principle at least, to have runtime indexing of the elements of a tuple by
generating a std::variant to refer to a tuple element at runtime. This is what `make_variant` and 
`make_reference_variant` do.

The std::variant provides a single class that identifies any element of the tuple at runtime. However, the index itself
can perform the same function, allowing us to do away with std::variants, if only we had a version of std::visit that
would accept tuples and element indices. This is what `visit_tuple` does. In this way, runtime dispatch to a function
given a tuple and a set of indices can be achieved in a single vtable lookup, which will be optimised out if it is
deducible at compile-time.

We also supply some convenience functions specifically for tuples of ranges and tuples of vectors.

See main.cpp for example usage.
