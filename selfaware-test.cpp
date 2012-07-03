#undef NDEBUG

#include "selfaware.hpp"
#include <array>
#include <cstring>
#include <cassert>
#include <iostream>

namespace selfaware_test {
    using namespace selfaware;
    using namespace std;

    SELFAWARE_IDENTIFIER(X)
    SELFAWARE_IDENTIFIER(Y)
    SELFAWARE_IDENTIFIER(Z)
    using TestStruct = Struct<X<int>, Y<float>, Z<char>>;
    static_assert(is_trivial<TestStruct>::value, "struct should be trivial");
    static_assert(is_same<decltype(TestStruct().X), int>::value, "decltype of x should be int");
    static_assert(is_same<decltype(TestStruct().Y), float>::value, "decltype of y should be float");
    static_assert(is_same<decltype(TestStruct().Z), char>::value, "decltype of z should be char");
    static_assert(is_same<TestStruct::type_of<X>, int>::value, "type_of x should be int");
    static_assert(is_same<TestStruct::type_of<Y>, float>::value, "type_of y should be float");
    static_assert(is_same<TestStruct::type_of<Z>, char>::value, "type_of z should be char");
    static_assert(is_convertible<TestStruct, tuple<int,float,char>>::value, "struct should be convertible to tuple");
    static_assert(is_convertible<tuple<int,float,char>, TestStruct>::value, "tuple should be convertible to struct");
    static_assert(TestStruct::size_of<X>() == sizeof(int), "size_of<x> should be sizeof(int)");
    static_assert(TestStruct::size_of<Y>() == sizeof(float), "size_of<y> should be sizeof(float)");
    static_assert(TestStruct::size_of<Z>() == sizeof(char), "size_of<z> should be sizeof(char)");
//        static_assert(TestStruct::offset_of<X>() != TestStruct::offset_of<Y>()
//                      && TestStruct::offset_of<Y>() != TestStruct::offset_of<Z>()
//                      && TestStruct::offset_of<Z>() != TestStruct::offset_of<X>(), "fields should have different offsets");
    static_assert(TestStruct(1, 2.0f, '3').X == 1, "struct should be constexpr constructible; X should be constexpr accessible");
    static_assert(TestStruct{1, 2.0f, '3'}.Z == '3', "struct should be constexpr ilist constructible; Z should be constexpr accessible");

    using TestEnum = Enum<unsigned, X, Y>;
    using TestEnum2 = Enum<unsigned, Y, X>;
    static_assert(is_trivial<TestEnum>::value, "enum should be trivial");
    static_assert(is_standard_layout<TestEnum>::value, "enum should be standard layout");
    static_assert(sizeof(TestEnum) == sizeof(unsigned), "enum should have same size as underlying type");
    static_assert(is_same<TestEnum::underlying_type, unsigned>::value, "underlying type of enum should be unsigned");
    static_assert(is_convertible<decltype(TestEnum::X), TestEnum>::value, "type of member should be convertible to enum");
    static_assert(!is_convertible<TestEnum, unsigned>::value, "enum should not be convertible to underlying type");
    static_assert(!is_convertible<unsigned, TestEnum>::value, "underlying type should not be convertible to enum");
    static_assert(!is_convertible<TestEnum, TestEnum2>::value, "different enums should not be convertible");
    static_assert(TestEnum(unsigned(TestEnum::X)) == TestEnum::X, "enum member should be explicitly convertible to underlying type and back");
    static_assert(TestEnum(unsigned(TestEnum::Y)) == TestEnum::Y, "enum member should be explicitly convertible to underlying type and back");
    //XFAIL static_assert(TestEnum::X != TestEnum::Y, "enum members have distinct values by default");

    SELFAWARE_IDENTIFIER(foo)
    SELFAWARE_IDENTIFIER(bar)
    SELFAWARE_IDENTIFIER(bas)
    using SomeStruct = Struct<foo<int>, bar<char>, bas<float>>;
    
    void testStructConstructionFromElements()
    {
        SomeStruct t(1, '2', 3.0f);
        assert(t.foo == 1);
        assert(t.bar == '2');
        assert(t.bas == 3.0f);

        SomeStruct u{4, '5', 6.0f};
        assert(u.foo == 4);
        assert(u.bar == '5');
        assert(u.bas == 6.0f);
    }

    using SomeArrayStruct = Struct<foo<array<int,2>>, bar<array<float,2>>>;

    void testStructConstructionFromArrayElements()
    {
        SomeArrayStruct x{{1,2},{3.0f, 4.0f}};
        assert(false);
    }

    void testStructConversionFromTuple()
    {
        tuple<int, char, float> x(7, '8', 9.0f);
        SomeStruct t = x;

        assert(t.foo == 7);
        assert(t.bar == '8');
        assert(t.bas == 9.0f);
    }

    void testStructConversionToTuple()
    {
        SomeStruct t{10, 'b', 12.0f};
        tuple<int, char,float> x = t;

        assert(get<0>(x) == 10);
        assert(get<1>(x) == 'b');
        assert(get<2>(x) == 12.0f);
    }

    void testStructApply()
    {
        SomeStruct t{13, 'd', 15.0f};
        t.apply([](int x, char y, float z) {
            assert(x == 13);
            assert(y == 'd');
            assert(z == 15.0f);
        });
    }

    void testStructOffsetOf()
    {
        size_t foo_offset = SomeStruct::offset_of<foo>();
        size_t bar_offset = SomeStruct::offset_of<bar>();
        size_t bas_offset = SomeStruct::offset_of<bas>();
        
        SomeStruct t(1, '2', 3.0f);
        
        int *fooPtr = reinterpret_cast<int*>(reinterpret_cast<char*>(&t) + foo_offset);
        assert(*fooPtr == 1);
        char *barPtr = reinterpret_cast<char*>(&t) + bar_offset;
        assert(*barPtr == '2');
        float *basPtr = reinterpret_cast<float*>(reinterpret_cast<char*>(&t) + bas_offset);
        assert(*basPtr == 3.0f);
    }
    
    template<typename T> struct test_trait;
    template<> struct test_trait<int> { static int value() { return 1; } };
    template<> struct test_trait<float> { static int value() { return 2; } };
    template<> struct test_trait<char> { static int value() { return 3; } };

    void testStructEachField()
    {
        SomeStruct::each_field<test_trait>([](char const *name, size_t offset, size_t size, int type) {
            if (strcmp(name, "foo") == 0) {
                assert(size == sizeof(int));
                assert(offset == SomeStruct::offset_of<foo>());
                assert(type == test_trait<int>::value());
            } else if (strcmp(name, "bar") == 0) {
                assert(size == sizeof(char));
                assert(offset == SomeStruct::offset_of<bar>());
                assert(type == test_trait<char>::value());
            } else if (strcmp(name, "bas") == 0) {
                assert(size == sizeof(float));
                assert(offset == SomeStruct::offset_of<bas>());
                assert(type == test_trait<float>::value());
            }
        });
    }
}

int main()
{
    using namespace selfaware_test;
    testStructConstructionFromElements();
    testStructConstructionFromArrayElements();
    testStructConversionFromTuple();
    testStructConversionToTuple();
    testStructApply();
    testStructOffsetOf();
    testStructEachField();
    std::cout << "ok!\n";
}
