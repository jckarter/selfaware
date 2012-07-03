# Self-aware structs in C++11

This library provides C++ data types with a user interface like C structs (trivial construction,
named accessors, strong types) but with tuple-like metaprogramming abilities, including the ability
to query field names as strings and get each field's offset, size, and type.

## Example

```c++
#include "selfaware.hpp"
#include <tuple>
#include <cstdio>

using namespace selfaware;
using namespace std;

// create a "selfaware identifier"; this is a class template that knows its own
// name and can be used with selfaware::Struct
SELFAWARE_IDENTIFIER(foo)
SELFAWARE_IDENTIFIER(bar)

// instantiate a selfaware struct type
using FooBar = Struct<foo<int>, bar<double>>;

// Struct<T...> looks like a struct to user code; it can be constructed using
// initializer lists, and its fields can be accessed using normal data member
// accessors
FooBar frob(int i)
{
    FooBar x = {1, 2.0};
    x.foo += i;
    x.bar += double(i);
    return x;
}

// However...

// Struct<T...> can also be converted to a tuple and back

tuple<int, double> to_tuple(FooBar const &x) { return x; }
FooBar from_tuple(tuple<int, double> const &x) { return x; }

// Struct<T...> can be unpacked and applied to functions
void show(int foo, double bar) { printf("%x %f\n", foo, bar); }

void showFooBar(FooBar const &fb) { fb.apply(show); }

// Struct<T...>'s field metadata can be iterated, including field name,
// offset, size, and type trait
template<typename> struct show_trait;
template<> struct show_trait<int> {
    static char const* value() { return "int"; }
};
template<> struct show_trait<double> {
    static char const* value() { return "double"; }
};

void showFields()
{
    printf("FooBar has the following fields:\n");
    FooBar::each_field<show_trait>(
        [](char const *fieldName, unsigned long offset, unsigned long size, char const *typeName) {
            printf("%s: offset %lu, size %lu, type %s\n", fieldName, offset, size, typeName);
        });
}
```

## License

BSD

## How to install

Place `selfaware.hpp` somewhere it can be included. The library is header-only. A
test suite is also included in `selfaware-test.cpp`, but this does not need to be
linked in with user code.

## Platforms

Tested with Clang 3.1 using libc++.

## Caveats

C++11 does not provide a standard mechanism for querying the offsets of fields within
non-standard-layout types, so this library relies on nonstandard behavior to do so.
