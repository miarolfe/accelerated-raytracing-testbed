// Copyright Mia Rolfe. All rights reserved.
#include <Catch2/catch.hpp>

#include <Core/ArenaAllocator.h>
#include <Core/Constants.h>

namespace ART
{

TEST_CASE("ArenaAllocator constructor initializes correctly", "[ArenaAllocator]")
{
    ArenaAllocator arena(ONE_KILOBYTE);

    // Should be able to allocate from a fresh arena
    void* allocation = arena.Alloc(16);
    REQUIRE(allocation != nullptr);
}

TEST_CASE("ArenaAllocator basic allocation", "[ArenaAllocator]")
{
    ArenaAllocator arena(ONE_KILOBYTE);

    void* allocation1 = arena.Alloc(16);
    REQUIRE(allocation1 != nullptr);

    void* allocation2 = arena.Alloc(32);
    REQUIRE(allocation2 != nullptr);

    // Pointers should be different
    REQUIRE(allocation1 != allocation2);
}

TEST_CASE("ArenaAllocator respects alignment", "[ArenaAllocator]")
{
    ArenaAllocator arena(ONE_KILOBYTE);

    SECTION("Default 16-byte alignment")
    {
        void* allocation = arena.Alloc(1);
        REQUIRE(allocation != nullptr);
        REQUIRE(reinterpret_cast<std::uintptr_t>(allocation) % 16 == 0);
    }

    SECTION("8-byte alignment")
    {
        void* allocation = arena.Alloc(1, 8);
        REQUIRE(allocation != nullptr);
        REQUIRE(reinterpret_cast<std::uintptr_t>(allocation) % 8 == 0);
    }

    SECTION("32-byte alignment")
    {
        void* allocation = arena.Alloc(1, 32);
        REQUIRE(allocation != nullptr);
        REQUIRE(reinterpret_cast<std::uintptr_t>(allocation) % 32 == 0);
    }

    SECTION("64-byte alignment")
    {
        void* allocation = arena.Alloc(1, 64);
        REQUIRE(allocation != nullptr);
        REQUIRE(reinterpret_cast<std::uintptr_t>(allocation) % 64 == 0);
    }
}

TEST_CASE("ArenaAllocator maintains alignment across multiple allocations", "[ArenaAllocator]")
{
    ArenaAllocator arena(ONE_KILOBYTE);

    // Allocate unaligned size
    void* allocation1 = arena.Alloc(15, 16);
    REQUIRE(allocation1 != nullptr);
    REQUIRE(reinterpret_cast<std::uintptr_t>(allocation1) % 16 == 0);

    // Next allocation should still be properly aligned
    void* allocation2 = arena.Alloc(15, 16);
    REQUIRE(allocation2 != nullptr);
    REQUIRE(reinterpret_cast<std::uintptr_t>(allocation2) % 16 == 0);
}

TEST_CASE("ArenaAllocator returns nullptr when out of capacity", "[ArenaAllocator]")
{
    ArenaAllocator arena(64);

    // Allocate most of the capacity
    void* allocation1 = arena.Alloc(32);
    REQUIRE(allocation1 != nullptr);

    // This should still fit
    void* allocation2 = arena.Alloc(16);
    REQUIRE(allocation2 != nullptr);

    // This should exceed capacity
    void* allocation3 = arena.Alloc(32);
    REQUIRE(allocation3 == nullptr);
}

TEST_CASE("ArenaAllocator returns nullptr when single allocation exceeds capacity", "[ArenaAllocator]")
{
    ArenaAllocator arena(64);

    void* allocation = arena.Alloc(128);
    REQUIRE(allocation == nullptr);
}

TEST_CASE("ArenaAllocator Clear resets allocator", "[ArenaAllocator]")
{
    ArenaAllocator arena(64);

    void* allocation1 = arena.Alloc(32);
    REQUIRE(allocation1 != nullptr);

    void* allocation2 = arena.Alloc(32);
    REQUIRE(allocation2 != nullptr);

    // Arena should be full
    void* allocation3 = arena.Alloc(16);
    REQUIRE(allocation3 == nullptr);

    // Clear and try again
    arena.Clear();

    void* allocation4 = arena.Alloc(32);
    REQUIRE(allocation4 != nullptr);

    // After clear, should allocate from beginning (same address as ptr1)
    REQUIRE(allocation4 == allocation1);
}

TEST_CASE("ArenaAllocator Create constructs objects", "[ArenaAllocator]")
{
    struct TestStruct
    {
    public:
        int m_a;
        double m_b;

        TestStruct(int x, double y) : m_a(x), m_b(y) {}
    };

    ArenaAllocator arena(ONE_KILOBYTE);

    TestStruct* test_struct = arena.Create<TestStruct>(22052003, 2205.2003);
    REQUIRE(test_struct != nullptr);
    REQUIRE(test_struct->m_a == 22052003);
    REQUIRE(test_struct->m_b == Approx(2205.2003));
}

TEST_CASE("ArenaAllocator Create uses correct alignment for types", "[ArenaAllocator]")
{
    struct AlignedTestStruct
    {
        alignas(32) double m_x;

        AlignedTestStruct(double x) : m_x(x) {}
    };

    ArenaAllocator arena(ONE_KILOBYTE);

    AlignedTestStruct* test_struct = arena.Create<AlignedTestStruct>(2205.2003);
    REQUIRE(test_struct != nullptr);
    REQUIRE(reinterpret_cast<std::uintptr_t>(test_struct) % 32 == 0);
    REQUIRE(test_struct->m_x == Approx(2205.2003));
}

TEST_CASE("ArenaAllocator Create returns nullptr when capacity exceeded", "[ArenaAllocator]")
{
    struct LargeTestStruct
    {
    public:
        char m_data[100];

        LargeTestStruct() {}
    };

    ArenaAllocator arena(64);

    LargeTestStruct* test_struct = arena.Create<LargeTestStruct>();
    REQUIRE(test_struct == nullptr);
}

TEST_CASE("ArenaAllocator Create multiple objects", "[ArenaAllocator]")
{
    struct SmallTestStruct
    {
    public:
        int m_x;
        int m_y;

        SmallTestStruct(int x, int y) : m_x(x), m_y(y) {}
    };

    ArenaAllocator arena(ONE_KILOBYTE);

    SmallTestStruct* test_struct_1 = arena.Create<SmallTestStruct>(1, 2);
    SmallTestStruct* test_struct_2 = arena.Create<SmallTestStruct>(3, 4);
    SmallTestStruct* test_struct_3 = arena.Create<SmallTestStruct>(5, 6);

    REQUIRE(test_struct_1 != nullptr);
    REQUIRE(test_struct_2 != nullptr);
    REQUIRE(test_struct_3 != nullptr);

    REQUIRE(test_struct_1->m_x == 1);
    REQUIRE(test_struct_1->m_y == 2);
    REQUIRE(test_struct_2->m_x == 3);
    REQUIRE(test_struct_2->m_y == 4);
    REQUIRE(test_struct_3->m_x == 5);
    REQUIRE(test_struct_3->m_y == 6);

    // All pointers should be distinct
    REQUIRE(test_struct_1 != test_struct_2);
    REQUIRE(test_struct_2 != test_struct_3);
    REQUIRE(test_struct_1 != test_struct_3);
}

// Edge case: zero-size allocations should be safe
TEST_CASE("ArenaAllocator handles zero-size allocation", "[ArenaAllocator]")
{
    ArenaAllocator arena(ONE_KILOBYTE);

    void* allocation1 = arena.Alloc(0);
    REQUIRE(allocation1 != nullptr);

    // Should still be able to allocate after zero-size allocation
    void* allocation2 = arena.Alloc(16);
    REQUIRE(allocation2 != nullptr);
}

TEST_CASE("ArenaAllocator alignment padding doesn't cause premature capacity exhaustion", "[ArenaAllocator]")
{
    ArenaAllocator arena(128);

    // Allocate with 1-byte alignment to offset subsequent allocations
    void* allocation1 = arena.Alloc(1, 1);
    REQUIRE(allocation1 != nullptr);

    // Allocate with 16-byte alignment - will need padding
    void* alloation2 = arena.Alloc(64, 16);
    REQUIRE(alloation2 != nullptr);
    REQUIRE(reinterpret_cast<std::uintptr_t>(alloation2) % 16 == 0);

    // Should still have room for another allocation
    void* allocation3 = arena.Alloc(32, 16);
    REQUIRE(allocation3 != nullptr);
}

} // namespace ART
