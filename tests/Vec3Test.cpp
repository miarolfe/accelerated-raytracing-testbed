// Copyright Mia Rolfe. All rights reserved.
#include "../external/Catch2/catch.hpp"

#include "../lib/Vec3.h"

namespace ART
{

TEST_CASE("Vec3 constructors", "[Vec3]")
{
    const Vec3 vec1;
    REQUIRE(vec1.m_x == Approx(0.0));
    REQUIRE(vec1.m_y == Approx(0.0));
    REQUIRE(vec1.m_z == Approx(0.0));

    const Vec3 vec2(2.0);
    REQUIRE(vec2.m_x == Approx(2.0));
    REQUIRE(vec2.m_y == Approx(2.0));
    REQUIRE(vec2.m_z == Approx(2.0));

    const Vec3 vec3(1.0, 2.0, 3.0);
    REQUIRE(vec3.m_x == Approx(1.0));
    REQUIRE(vec3.m_y == Approx(2.0));
    REQUIRE(vec3.m_z == Approx(3.0));
}

TEST_CASE("Vec3 index operators", "[Vec3]")
{
    Vec3 vec(1.0, 2.0, 3.0);

    REQUIRE(vec[0] == Approx(1.0));
    REQUIRE(vec[1] == Approx(2.0));
    REQUIRE(vec[2] == Approx(3.0));

    vec[0] = 4.0;
    vec[1] = 5.0;
    vec[2] = 6.0;

    REQUIRE(vec.m_x == Approx(4.0));
    REQUIRE(vec.m_y == Approx(5.0));
    REQUIRE(vec.m_z == Approx(6.0));
}

TEST_CASE("Vec3 sign flip", "[Vec3]")
{
    const Vec3 vec(1.0, -2.0, 3.0);
    const Vec3 sign_flipped_vec = -vec;

    REQUIRE(sign_flipped_vec.m_x == Approx(-1.0));
    REQUIRE(sign_flipped_vec.m_y == Approx(2.0));
    REQUIRE(sign_flipped_vec.m_z == Approx(-3.0));
}

TEST_CASE("Vec3 arithmetic operators", "[Vec3]")
{
    const Vec3 vec1(1.0, 2.0, 3.0);
    const Vec3 vec2(4.0, 5.0, 6.0);

    const Vec3 sum = vec1 + vec2;
    REQUIRE(sum.m_x == Approx(5.0));
    REQUIRE(sum.m_y == Approx(7.0));
    REQUIRE(sum.m_z == Approx(9.0));

    const Vec3 diff = vec2 - vec1;
    REQUIRE(diff.m_x == Approx(3.0));
    REQUIRE(diff.m_y == Approx(3.0));
    REQUIRE(diff.m_z == Approx(3.0));

    const Vec3 component_wise_multiplied = vec1 * vec2;
    REQUIRE(component_wise_multiplied.m_x == Approx(4.0));
    REQUIRE(component_wise_multiplied.m_y == Approx(10.0));
    REQUIRE(component_wise_multiplied.m_z == Approx(18.0));

    const Vec3 scaled = vec1 * 2.0;
    REQUIRE(scaled.m_x == Approx(2.0));
    REQUIRE(scaled.m_y == Approx(4.0));
    REQUIRE(scaled.m_z == Approx(6.0));

    const Vec3 divided = scaled / 2.0;
    REQUIRE(divided.m_x == Approx(1.0));
    REQUIRE(divided.m_y == Approx(2.0));
    REQUIRE(divided.m_z == Approx(3.0));
}

TEST_CASE("Vec3 repeated assignment", "[Vec3]")
{
    Vec3 vec1(1.0, 2.0, 3.0);
    const Vec3 vec2(1.0, 1.0, 1.0);

    vec1 += vec2;
    REQUIRE(vec1.m_x == Approx(2.0));
    REQUIRE(vec1.m_y == Approx(3.0));
    REQUIRE(vec1.m_z == Approx(4.0));

    vec1 *= 2.0;
    REQUIRE(vec1.m_x == Approx(4.0));
    REQUIRE(vec1.m_y == Approx(6.0));
    REQUIRE(vec1.m_z == Approx(8.0));

    vec1 /= 2.0;
    REQUIRE(vec1.m_x == Approx(2.0));
    REQUIRE(vec1.m_y == Approx(3.0));
    REQUIRE(vec1.m_z == Approx(4.0));
}

TEST_CASE("Vec3 length and length squared", "[Vec3]")
{
    Vec3 vec(3.0, 4.0, 0.0);

    REQUIRE(vec.LengthSquared() == Approx(25.0));
    REQUIRE(vec.Length() == Approx(5.0));
}

TEST_CASE("Vec3 NearZero", "[Vec3]")
{
    Vec3 vec1(1e-10, -1e-10, 1e-12);
    REQUIRE(vec1.NearZero());

    Vec3 vec2(1e-4, 0.0, 0.0);
    REQUIRE_FALSE(vec2.NearZero());
}

TEST_CASE("Vec3 Dot product", "[Vec3]")
{
    Vec3 vec1(1.0, 2.0, 3.0);
    Vec3 vec2(4.0, -5.0, 6.0);

    REQUIRE(Dot(vec1, vec2) == Approx(12.0));
}

TEST_CASE("Vec3 Cross product", "[Vec3]")
{
    const Vec3 vec1(1.0, 0.0, 0.0);
    const Vec3 vec2(0.0, 1.0, 0.0);

    const Vec3 vec3 = Cross(vec1, vec2);
    REQUIRE(vec3.m_x == Approx(0.0));
    REQUIRE(vec3.m_y == Approx(0.0));
    REQUIRE(vec3.m_z == Approx(1.0));
}

TEST_CASE("Vec3 Normalised", "[Vec3]")
{
    Vec3 vec(0.0, 3.0, 4.0);
    Vec3 normalised_vec = Normalised(vec);

    REQUIRE(normalised_vec.Length() == Approx(1.0));
    REQUIRE(normalised_vec.m_y == Approx(3.0 / 5.0));
    REQUIRE(normalised_vec.m_z == Approx(4.0 / 5.0));
}

TEST_CASE("Vec3 Reflect", "[Vec3]")
{
    Vec3 v(1.0, -1.0, 0.0);
    Vec3 n(0.0, 1.0, 0.0);

    Vec3 r = Reflect(v, n);
    REQUIRE(r.m_x == Approx(1.0));
    REQUIRE(r.m_y == Approx(1.0));
    REQUIRE(r.m_z == Approx(0.0));
}

TEST_CASE("Vec3 Refract (normal incidence)", "[Vec3]")
{
    Vec3 uv(0.0, -1.0, 0.0);
    Vec3 n(0.0, 1.0, 0.0);

    Vec3 r = Refract(uv, n, 1.0);
    REQUIRE(r.m_x == Approx(0.0));
    REQUIRE(r.m_y == Approx(-1.0));
    REQUIRE(r.m_z == Approx(0.0));
}

TEST_CASE("Vec3 Random vectors are within expected bounds", "[Vec3][Random]")
{
    Vec3 vec = Vec3::Random(-1.0, 1.0);

    REQUIRE(vec.m_x >= -1.0);
    REQUIRE(vec.m_x <  1.0);
    REQUIRE(vec.m_y >= -1.0);
    REQUIRE(vec.m_y <  1.0);
    REQUIRE(vec.m_z >= -1.0);
    REQUIRE(vec.m_z <  1.0);
}

} // namespace ART
