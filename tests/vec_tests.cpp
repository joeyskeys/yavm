#include <catch2/catch_all.hpp>

#include <yavl/yavl.h>

using namespace yavl;

using Catch::Approx;

#define PROPER_EQUAL(EXPR, VALUE, VAR)                                  \
    {                                                                   \
        if constexpr (std::is_floating_point_v<typename decltype(VAR)::Scalar>) \
            REQUIRE(EXPR == Approx(VALUE));                             \
        else                                                            \
            REQUIRE(EXPR == VALUE);                                     \
    }

template <typename V>
void ctor_test(const V& v, const typename V::Scalar s) {
    if constexpr (std::is_floating_point_v<typename V::Scalar>) {
        REQUIRE(v.x == Approx(s));
        REQUIRE(v.r == Approx(s));
        REQUIRE(v.y == Approx(s));
        REQUIRE(v.g == Approx(s));
        if constexpr (V::Size > 2) {
            REQUIRE(v.z == Approx(s));
            REQUIRE(v.b == Approx(s));
        }
        if constexpr (V::Size > 3) {
            REQUIRE(v.w == Approx(s));
            REQUIRE(v.a == Approx(s));
        }
    }
    else {
        REQUIRE(v.x == s);
        REQUIRE(v.r == s);
        REQUIRE(v.y == s);
        REQUIRE(v.g == s);
        if constexpr (V::Size > 2) {
            REQUIRE(v.z == s);
            REQUIRE(v.b == s);
        }
        if constexpr (V::Size > 3) {
            REQUIRE(v.w == s);
            REQUIRE(v.a == s);
        }
    }
}

template <typename V, typename ...Ts>
void check_vec_components(const V& v, Ts... args) {
    static_assert(sizeof...(args) == V::Size);
    std::array<typename V::Scalar, V::Size> arg_array{ static_cast<typename V::Scalar>(args)... };
    if constexpr (std::is_floating_point_v<typename V::Scalar>) {
        for (int i = 0; i < V::Size; ++i)
            REQUIRE(v[i] == Approx(arg_array[i]));
    }
    else {
        for (int i = 0; i < V::Size; ++i)
            REQUIRE(v[i] == arg_array[i]);
    }
}

TEMPLATE_PRODUCT_TEST_CASE("Vec tests", "[vec]", (Vec2, Vec3, Vec4), (float, double, int, uint32_t)) {
    // Ctor tests
    TestType vec0;
    TestType vec1{1};
    TestType vec2{2};
    TestType vec3, vec4;
    TestType vec5{1};
    if constexpr (vec0.Size == 2) {
        vec3 = {1, 2};
        vec4 = {5, 6};
    }
    else if constexpr (vec0.Size == 3) {
        vec3 = {1, 2, 3};
        vec4 = {5, 6, 7};
    }
    else if constexpr (vec0.Size == 4) {
        vec3 = {1, 2, 3, 4};
        vec4 = {5, 6, 7, 8};
    }
    else
        static_assert(vec0.Size < 5, "Should not reach here");

    SECTION("Ctor tests") {
        ctor_test(vec0, 0);
        ctor_test(vec1, 1);

        if constexpr (vec3.Size == 2)
            check_vec_components(vec3, 1, 2);
        else if constexpr (vec3.Size == 3)
            check_vec_components(vec3, 1, 2, 3);
        else if constexpr (vec3.Size == 4)
            check_vec_components(vec3, 1, 2, 3, 4);
        else
            static_assert(vec3.Size < 5, "Should not reach here");
    }

    SECTION("Operator tests") {
        auto vec5 = vec1 + vec2;
        PROPER_EQUAL(vec5.x, 3, vec5);
        PROPER_EQUAL(vec5.y, 3, vec5);

        vec5 = vec2 - vec1;
        PROPER_EQUAL(vec5.x, 1, vec5);
        PROPER_EQUAL(vec5.y, 1, vec5);

        vec5 = vec2 * vec3;
        PROPER_EQUAL(vec5.x, 2, vec5);
        PROPER_EQUAL(vec5.y, 4, vec5);
        if constexpr (vec5.Size > 2)
            PROPER_EQUAL(vec5.z, 6, vec5);
        if constexpr (vec5.Size > 3)
            PROPER_EQUAL(vec5.w, 8, vec5);

        if constexpr (std::is_floating_point_v<typename decltype(vec5)::Scalar>) {
            vec5 = vec3 / vec2;
            REQUIRE(vec5.x == Approx(0.5));
            REQUIRE(vec5.y == Approx(1));
        }
    }

    SECTION("Misc tests") {
        if constexpr (vec3.Size == 2) {
            auto vec5 = vec3.template shuffle<1, 0>();
            check_vec_components(vec5, 2, 1);
            auto vec6 = vec4.shuffle(1, 0);
            check_vec_components(vec6, 6, 5);
        }
        else if constexpr (vec3.Size == 3) {
            auto vec5 = vec3.template shuffle<2, 1, 0>();
            check_vec_components(vec5, 3, 2, 1);
            auto vec6 = vec4.shuffle(2, 1, 0);
            check_vec_components(vec6, 7, 6, 5);
        }
        else if constexpr (vec3.Size == 4) {
            auto vec5 = vec3.template shuffle<3, 2, 1, 0>();
            check_vec_components(vec5, 4, 3, 2, 1);
            auto vec6 = vec4.shuffle(3, 2, 1, 0);
            check_vec_components(vec6, 8, 7, 6, 5);
        }
    }

    SECTION("Geo tests") {
        if constexpr (TestType::Size == 2) {
            auto dot_val = vec1.dot(vec3);
            PROPER_EQUAL(dot_val, 3, vec1);
            auto cross_val = vec1.cross(vec3);
            PROPER_EQUAL(cross_val, 1, vec1);
        }
        else if constexpr (TestType::Size == 3) {
            auto dot_val = vec1.dot(vec3);
            PROPER_EQUAL(dot_val, 6, vec1);
            auto cross_val = vec1.cross(vec3);
            check_vec_components(cross_val, 1, -2, 1);
        }
        else if constexpr (TestType::Size == 4) {
            auto dot_val = vec1.dot(vec3);
            PROPER_EQUAL(dot_val, 10, vec1);
        }
    }

    SECTION("Math tests") {
        if constexpr (TestType::Size == 2) {
            auto length_sqr = vec1.length_squared();
            PROPER_EQUAL(length_sqr, 2, vec1);
            auto length = vec1.length();
            REQUIRE(length == Approx(1.41421356));
            auto sum = vec1.sum();
            PROPER_EQUAL(sum, 2, vec1);
            auto sqr = vec3.square();
            PROPER_EQUAL(sqr.x, 1, vec3);
            PROPER_EQUAL(sqr.y, 4, vec3);
            if constexpr (std::is_floating_point_v<typename TestType::Scalar>) {
                auto rcp = vec3.rcp();
                auto sqrt = vec3.sqrt();
                auto rsqrt = vec3.rsqrt();
                TestType vec5{4};
                auto lerp = vec2.lerp(vec5, 0.5);
                REQUIRE(rcp.x == Approx(1));
                REQUIRE(rcp.y == Approx(0.5));
                REQUIRE(sqrt.x == Approx(1));
                REQUIRE(sqrt.y == Approx(1.4142135624));
                REQUIRE(rsqrt.x == Approx(1));
                REQUIRE(rsqrt.y == Approx(0.7071067812));
            }
        }
    }

    SECTION("Compare tests") {
        REQUIRE(vec1 != vec2)
        REQUIRE(vec1 == vec5)
    }

    SECTION("Vectorization tests") {
        //if constexpr (TestType::Size > 2)
            //REQUIRE(TestType::vectorized == true);

        REQUIRE(Vec<int, 4>::vectorized == true);
    }
}