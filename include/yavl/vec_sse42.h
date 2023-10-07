#pragma once

#include <concepts>

namespace yavl
{

template <>
struct alignas(16) Vec<float, 4> {
    YAVL_VEC_ALIAS(float, 4)

    static constexpr bool vectorized = true;

    union {
        struct {
            Scalar x, y, z, w;
        };
        struct {
            Scalar r, g, b, a;
        };
        
        std::array<Scalar, Size> arr;
        __m128 m;
    };

    // Ctors
    Vec() : m(_mm_set1_ps(0)) {}

    template <typename V>
        requires std::default_initializable<V> && std::convertible_to<V, Scalar>
    Vec(V v) : m(_mm_set1_ps(static_cast<Scalar>(v))) {}

    template <typename ...Ts>
        requires (std::default_initializable<Ts> && ...) && (std::convertible_to<Ts, Scalar> && ...)
    Vec(Ts... args) {
            m = _mm_setr_ps(args...);
    }

    Vec(const __m128 val) : m(val) {}

    // Operators
    YAVL_DEFINE_VEC_INDEX_OP
    
#define OP_VEC_EXPRS(OP, NAME)                                          \
    return Vec(_mm_##NAME##_ps(m, v.m));

#define OP_VEC_ASSIGN_EXPRS(OP, NAME)                                   \
    m = _mm_##NAME##_ps(m, v.m);                                        \
    return *this;

#define OP_SCALAR_EXPRS(OP, NAME)                                       \
    auto vv = _mm_set1_ps(v);                                           \
    return Vec(_mm_##NAME##_ps(m, vv));

#define OP_SCALAR_ASSIGN_EXPRS(OP, NAME)                                \
    auto vv = _mm_set1_ps(v);                                           \
    m = _mm_##NAME##_ps(m, vv);                                         \
    return *this;

#define OP_FRIEND_SCALAR_EXPRS(OP, NAME)                                \
    auto vv = _mm_set1_ps(s);                                           \
    return Vec(_mm_##NAME##_ps(vv, v.m));

    YAVL_DEFINE_OP(+, add)
    YAVL_DEFINE_OP(-, sub)
    YAVL_DEFINE_OP(*, mul)
    YAVL_DEFINE_OP(/, div)

#undef OP_VEC_EXPRS
#undef OP_VEC_ASSIGN_EXPRS
#undef OP_SCALAR_EXPRS
#undef OP_SCALAR_ASSING_EXPRS
#undef OP_FRIEND_SCALAR_EXPRS

#define GEO_DOT_EXPRS                                                   \
    {                                                                   \
        return _mm_cvtss_f32(_mm_dp_ps(m, b.m, 0b11110001));            \
    }

    YAVL_DEFINE_GEO_FUNCS

#undef GEO_DOT_EXPRS

#define MATH_ABS_EXPRS                                                  \
    {                                                                   \
        /* Bitwise not with -0.f get the 0x7fff mask, bitwise and set */\
        /* the sign bit to zero hence abs for the floating point */     \
        return Vec(_mm_andnot_ps(_mm_set1_ps(-0.f), m));                \
    }

#define MATH_SUM_EXPRS                                                  \
    {                                                                   \
        auto t1 = _mm_hadd_ps(m, m);                                    \
        auto t2 = _mm_hadd_ps(t1, t1);                                  \
        return _mm_cvtss_f32(t2);                                       \
    }

#define MATH_RCP_EXPRS                                                  \
    {                                                                   \
        
    }

#define MATH_SQRT_EXPRS                                                 \
    {                                                                   \
        return Vec(_mm_sqrt_ps(m));                                     \
    }

#if defined(YAVL_X86_FMA)
#define MULADD(RET, A, B, C) RET = _mm_fmadd_ps(A, B, C)
#else
#define MULADD(RET, A, B, C) auto tmul = _mm_mul_ps(A, B), RET = _mm_add_ps(tmul, C)
#endif

#define MATH_LERP_SCALAR_EXPRS                                          \
    {                                                                   \
        auto vomt = _mm_set1_ps(1 - t);                                 \
        auto vt = _mm_set1_ps(t);                                       \
        auto t1 = _mm_mul_ps(b.m, vt);                                  \
        MULADD(auto ret, vomt, m, t1);                                  \
        return Vec(ret);                                                \
    }

#define MATH_LERP_VEC_EXPRS                                             \
    {                                                                   \
        Vec vomt = 1 - t;                                               \
        auto t1 = _mm_mul_ps(b.m, t.m);                                 \
        MULADD(auto ret, vomt.m, m, t1);                                \
        return Vec(ret);                                                \
    }

    YAVL_DEFINE_MATH_FUNCS

#undef MATH_ABS_EXPRS
#undef MATH_SUM_EXPRS
#undef MATH_SQRT_EXPRS
#undef MATH_EXP_EXPRS
#undef MATH_POW_EXPRS
#undef MATH_LERP_EXPRS
};

}