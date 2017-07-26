//=======================================================================
// Copyright (c) 2014-2017 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

// The idea of the GEMM kernels is largely inspired by the kernels in Blaze by
// Klaus Igleberg

namespace etl {

namespace impl {

namespace vec {

/*!
 * \brief Optimized version of GEMM for assignment of a small
 * Column-Major Matrix - Row Major Matrix to a Row Major Matrix.
 *
 * \param a The lhs matrix
 * \param b The rhs matrix
 * \param c The result matrix
 */
template <typename V, typename T>
void gemm_tn_small_kernel_rr(const T* a, const T* b, T* c, size_t M, size_t N, size_t K) {
    using vec_type = V;

    static constexpr size_t vec_size = vec_type::template traits<T>::size;

    const auto j_end = N & (size_t(-vec_size));

    size_t j = 0;

    for(; j + 7 * vec_size < j_end; j += 8 * vec_size){
        size_t i = 0;

        for (; i < M; i++) {
            auto r11 = vec_type::template zero<T>();
            auto r12 = vec_type::template zero<T>();
            auto r13 = vec_type::template zero<T>();
            auto r14 = vec_type::template zero<T>();
            auto r15 = vec_type::template zero<T>();
            auto r16 = vec_type::template zero<T>();
            auto r17 = vec_type::template zero<T>();
            auto r18 = vec_type::template zero<T>();

            for (size_t k = 0; k < K; ++k) {
                auto a1 = vec_type::set(a[(i + 0) + k * M]);

                auto b1 = vec_type::loadu(b + k * N + j + 0 * vec_size);
                auto b2 = vec_type::loadu(b + k * N + j + 1 * vec_size);
                auto b3 = vec_type::loadu(b + k * N + j + 2 * vec_size);
                auto b4 = vec_type::loadu(b + k * N + j + 3 * vec_size);
                auto b5 = vec_type::loadu(b + k * N + j + 4 * vec_size);
                auto b6 = vec_type::loadu(b + k * N + j + 5 * vec_size);
                auto b7 = vec_type::loadu(b + k * N + j + 6 * vec_size);
                auto b8 = vec_type::loadu(b + k * N + j + 7 * vec_size);

                r11 = vec_type::fmadd(a1, b1, r11);
                r12 = vec_type::fmadd(a1, b2, r12);
                r13 = vec_type::fmadd(a1, b3, r13);
                r14 = vec_type::fmadd(a1, b4, r14);
                r15 = vec_type::fmadd(a1, b5, r15);
                r16 = vec_type::fmadd(a1, b6, r16);
                r17 = vec_type::fmadd(a1, b7, r17);
                r18 = vec_type::fmadd(a1, b8, r18);
            }

            vec_type::storeu(c + (i + 0) * N + j + 0 * vec_size, r11);
            vec_type::storeu(c + (i + 0) * N + j + 1 * vec_size, r12);
            vec_type::storeu(c + (i + 0) * N + j + 2 * vec_size, r13);
            vec_type::storeu(c + (i + 0) * N + j + 3 * vec_size, r14);
            vec_type::storeu(c + (i + 0) * N + j + 4 * vec_size, r15);
            vec_type::storeu(c + (i + 0) * N + j + 5 * vec_size, r16);
            vec_type::storeu(c + (i + 0) * N + j + 6 * vec_size, r17);
            vec_type::storeu(c + (i + 0) * N + j + 7 * vec_size, r18);
        }
    }

    for(; j + 3 * vec_size < j_end; j += 4 * vec_size){
        size_t i = 0;

        for (; i + 1 < M; i += 2) {
            auto r11 = vec_type::template zero<T>();
            auto r21 = vec_type::template zero<T>();

            auto r12 = vec_type::template zero<T>();
            auto r22 = vec_type::template zero<T>();

            auto r13 = vec_type::template zero<T>();
            auto r23 = vec_type::template zero<T>();

            auto r14 = vec_type::template zero<T>();
            auto r24 = vec_type::template zero<T>();

            for (size_t k = 0; k < K; ++k) {
                auto a1 = vec_type::set(a[(i + 0) + k * M]);
                auto a2 = vec_type::set(a[(i + 1) + k * M]);

                auto b1 = vec_type::loadu(b + k * N + j + 0 * vec_size);
                auto b2 = vec_type::loadu(b + k * N + j + 1 * vec_size);
                auto b3 = vec_type::loadu(b + k * N + j + 2 * vec_size);
                auto b4 = vec_type::loadu(b + k * N + j + 3 * vec_size);

                r11 = vec_type::fmadd(a1, b1, r11);
                r21 = vec_type::fmadd(a2, b1, r21);

                r12 = vec_type::fmadd(a1, b2, r12);
                r22 = vec_type::fmadd(a2, b2, r22);

                r13 = vec_type::fmadd(a1, b3, r13);
                r23 = vec_type::fmadd(a2, b3, r23);

                r14 = vec_type::fmadd(a1, b4, r14);
                r24 = vec_type::fmadd(a2, b4, r24);
            }

            vec_type::storeu(c + (i + 0) * N + j + 0 * vec_size, r11);
            vec_type::storeu(c + (i + 1) * N + j + 0 * vec_size, r21);

            vec_type::storeu(c + (i + 0) * N + j + 1 * vec_size, r12);
            vec_type::storeu(c + (i + 1) * N + j + 1 * vec_size, r22);

            vec_type::storeu(c + (i + 0) * N + j + 2 * vec_size, r13);
            vec_type::storeu(c + (i + 1) * N + j + 2 * vec_size, r23);

            vec_type::storeu(c + (i + 0) * N + j + 3 * vec_size, r14);
            vec_type::storeu(c + (i + 1) * N + j + 3 * vec_size, r24);
        }

        for (; i < M; i++) {
            auto r11 = vec_type::template zero<T>();
            auto r12 = vec_type::template zero<T>();
            auto r13 = vec_type::template zero<T>();
            auto r14 = vec_type::template zero<T>();

            for (size_t k = 0; k < K; ++k) {
                auto a1 = vec_type::set(a[(i + 0) + k * M]);

                auto b1 = vec_type::loadu(b + k * N + j + 0 * vec_size);
                auto b2 = vec_type::loadu(b + k * N + j + 1 * vec_size);
                auto b3 = vec_type::loadu(b + k * N + j + 2 * vec_size);
                auto b4 = vec_type::loadu(b + k * N + j + 3 * vec_size);

                r11 = vec_type::fmadd(a1, b1, r11);
                r12 = vec_type::fmadd(a1, b2, r12);
                r13 = vec_type::fmadd(a1, b3, r13);
                r14 = vec_type::fmadd(a1, b4, r14);
            }

            vec_type::storeu(c + (i + 0) * N + j + 0 * vec_size, r11);
            vec_type::storeu(c + (i + 0) * N + j + 1 * vec_size, r12);
            vec_type::storeu(c + (i + 0) * N + j + 2 * vec_size, r13);
            vec_type::storeu(c + (i + 0) * N + j + 3 * vec_size, r14);
        }
    }

    for(; j + 1 * vec_size < j_end; j += 2 * vec_size){
        size_t i = 0;

        for (; i + 1 < M; i += 2) {
            auto r11 = vec_type::template zero<T>();
            auto r21 = vec_type::template zero<T>();

            auto r12 = vec_type::template zero<T>();
            auto r22 = vec_type::template zero<T>();

            for (size_t k = 0; k < K; ++k) {
                auto a1 = vec_type::set(a[(i + 0) + k * M]);
                auto a2 = vec_type::set(a[(i + 1) + k * M]);

                auto b1 = vec_type::loadu(b + k * N + j + 0 * vec_size);
                auto b2 = vec_type::loadu(b + k * N + j + 1 * vec_size);

                r11 = vec_type::fmadd(a1, b1, r11);
                r21 = vec_type::fmadd(a2, b1, r21);

                r12 = vec_type::fmadd(a1, b2, r12);
                r22 = vec_type::fmadd(a2, b2, r22);
            }

            vec_type::storeu(c + (i + 0) * N + j + 0 * vec_size, r11);
            vec_type::storeu(c + (i + 1) * N + j + 0 * vec_size, r21);

            vec_type::storeu(c + (i + 0) * N + j + 1 * vec_size, r12);
            vec_type::storeu(c + (i + 1) * N + j + 1 * vec_size, r22);
        }

        for (; i < M; i++) {
            auto r11 = vec_type::template zero<T>();
            auto r12 = vec_type::template zero<T>();

            for (size_t k = 0; k < K; ++k) {
                auto a1 = vec_type::set(a[(i + 0) + k * M]);

                auto b1 = vec_type::loadu(b + k * N + j + 0 * vec_size);
                auto b2 = vec_type::loadu(b + k * N + j + 1 * vec_size);

                r11 = vec_type::fmadd(a1, b1, r11);
                r12 = vec_type::fmadd(a1, b2, r12);
            }

            vec_type::storeu(c + (i + 0) * N + j + 0 * vec_size, r11);
            vec_type::storeu(c + (i + 0) * N + j + 1 * vec_size, r12);
        }
    }

    for(; j < j_end; j += vec_size){
        size_t i = 0;

        for (; i + 1 < M; i += 2) {
            auto r11 = vec_type::template zero<T>();
            auto r21 = vec_type::template zero<T>();

            for (size_t k = 0; k < K; ++k) {
                auto a1 = vec_type::set(a[(i + 0) + k * M]);
                auto a2 = vec_type::set(a[(i + 1) + k * M]);

                auto b1 = vec_type::loadu(b + k * N + j + 0 * vec_size);

                r11 = vec_type::fmadd(a1, b1, r11);
                r21 = vec_type::fmadd(a2, b1, r21);
            }

            vec_type::storeu(c + (i + 0) * N + j + 0 * vec_size, r11);
            vec_type::storeu(c + (i + 1) * N + j + 0 * vec_size, r21);
        }

        if (i < M) {
            auto r11 = vec_type::template zero<T>();

            for (size_t k = 0; k < K; ++k) {
                auto a1 = vec_type::set(a[i + k * M]);

                auto b1 = vec_type::loadu(b + k * N + j + 0 * vec_size);

                r11 = vec_type::fmadd(a1, b1, r11);
            }

            vec_type::storeu(c + i * N + j + 0 * vec_size, r11);
        }
    }

    for (; j < N; ++j) {
        size_t i = 0;

        for (; i + 1 < M; i += 2) {
            auto r1 = T();
            auto r2 = T();

            for (size_t k = 0; k < K; ++k) {
                r1 += a[(i + 0) + k * M] * b[k * N + j];
                r2 += a[(i + 1) + k * M] * b[k * N + j];
            }

            c[(i + 0) * N + j] = r1;
            c[(i + 1) * N + j] = r2;
        }

        if (i < M) {
            auto r1 = T();

            for (size_t k = 0; k < K; ++k) {
                r1 += a[i + k * M] * b[k * N + j];
            }

            c[i * N + j] = r1;
        }
    }
}

/*!
 * \brief Optimized version of GEMM for assignment of a
 * Column-Major Matrix - Row Major Matrix to a Row Major Matrix.
 *
 * \param a The lhs matrix (row major)
 * \param b The rhs matrix (transposed row major)
 * \param c The result matrix (row major)
 */
template <typename A, typename B, typename C, cpp_enable_if((all_row_major<A, B, C>::value))>
void gemm_tn(A&& a, B&& b, C&& c) {
    cpp_assert(vec_enabled, "At least one vector mode must be enabled for impl::VEC");

    a.ensure_cpu_up_to_date();
    b.ensure_cpu_up_to_date();

    const size_t M = etl::columns(a); // rows(trans(A)) = rows(C)
    const size_t N = etl::columns(b); // columns (B) = columns(C)
    const size_t K = etl::rows(a);    // columns(trans(A)) = rows(B)

    //if(etl::size(b) <= gemm_rr_small_threshold){
        gemm_tn_small_kernel_rr<default_vec>(a.memory_start(), b.memory_start(), c.memory_start(), M, N, K);
    //} else {
        //gemm_tn_large_kernel_rr<default_vec>(a.memory_start(), b.memory_start(), c.memory_start(), M, N, K);
    //}

    c.invalidate_gpu();
}

} //end of namespace vec
} //end of namespace impl
} //end of namespace etl
