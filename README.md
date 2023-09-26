# PolymorphicCollection
A fast C++ collection of objects of differing type, with runtime typing via std::variant and std::visit.

This is useful when:
 * you want a collection of objects of different, unrelated types (i.e. no shared base class)
 * the type of an accessed element may not be known at compile time (e.g. a randomly selected element)
 * the types of the elements all belong to a set of types known at compile time.

See main.cpp for example usage.

WARNING: this is a work in progress.
