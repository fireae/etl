//=======================================================================
// Copyright (c) 2014-2017 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*!
 * \file
 * \brief EGBLAS wrappers for the axdy operation.
 */

#pragma once

#ifdef ETL_EGBLAS_MODE

#include "etl/impl/cublas/cuda.hpp"

#include <egblas.hpp>

#endif

namespace etl {

namespace impl {

namespace egblas {

#ifdef EGBLAS_HAS_SAXDY

static constexpr bool has_saxdy = true;

/*!
 * \brief Wrappers for single-precision egblas axdy operation
 * \param n The size of the vector
 * \param alpha The scaling factor alpha
 * \param A The memory of the vector a
 * \param lda The leading dimension of a
 * \param B The memory of the vector b
 * \param ldb The leading dimension of b
 */
inline void axdy(size_t n, float* alpha, float* A , size_t lda, float* B , size_t ldb){
    egblas_saxdy(n, *alpha, A, lda, B, ldb);
}

#else

static constexpr bool has_saxdy = false;

#endif

#ifdef EGBLAS_HAS_DAXDY

static constexpr bool has_daxdy = true;

/*!
 * \brief Wrappers for double-precision egblas axdy operation
 * \param n The size of the vector
 * \param alpha The scaling factor alpha
 * \param A The memory of the vector a
 * \param lda The leading dimension of a
 * \param B The memory of the vector b
 * \param ldb The leading dimension of b
 */
inline void axdy(size_t n, double* alpha, double* A , size_t lda, double* B , size_t ldb){
    egblas_daxdy(n, *alpha, A, lda, B, ldb);
}

#else

static constexpr bool has_daxdy = false;

#endif

#ifdef EGBLAS_HAS_CAXDY

static constexpr bool has_caxdy = true;

/*!
 * \brief Wrappers for complex single-precision egblas axdy operation
 * \param n The size of the vector
 * \param alpha The scaling factor alpha
 * \param A The memory of the vector a
 * \param lda The leading dimension of a
 * \param B The memory of the vector b
 * \param ldb The leading dimension of b
 */
inline void axdy(size_t n, std::complex<float>* alpha, std::complex<float>* A , size_t lda, std::complex<float>* B , size_t ldb){
    egblas_caxdy(n, *reinterpret_cast<cuComplex*>(alpha), reinterpret_cast<cuComplex*>(A), lda, reinterpret_cast<cuComplex*>(B), ldb);
}

/*!
 * \brief Wrappers for complex single-precision egblas axdy operation
 * \param n The size of the vector
 * \param alpha The scaling factor alpha
 * \param A The memory of the vector a
 * \param lda The leading dimension of a
 * \param B The memory of the vector b
 * \param ldb The leading dimension of b
 */
inline void axdy(size_t n, etl::complex<float>* alpha, etl::complex<float>* A , size_t lda, etl::complex<float>* B , size_t ldb){
    egblas_caxdy(n, *reinterpret_cast<cuComplex*>(alpha), reinterpret_cast<cuComplex*>(A), lda, reinterpret_cast<cuComplex*>(B), ldb);
}

#else

static constexpr bool has_caxdy = false;

#endif

#ifdef EGBLAS_HAS_ZAXDY

static constexpr bool has_zaxdy = true;

/*!
 * \brief Wrappers for complex double-precision egblas axdy operation
 * \param n The size of the vector
 * \param alpha The scaling factor alpha
 * \param A The memory of the vector a
 * \param lda The leading dimension of a
 * \param B The memory of the vector b
 * \param ldb The leading dimension of b
 */
inline void axdy(size_t n, std::complex<double>* alpha, std::complex<double>* A , size_t lda, std::complex<double>* B , size_t ldb){
    egblas_zaxdy(n, *reinterpret_cast<cuDoubleComplex*>(alpha), reinterpret_cast<cuDoubleComplex*>(A), lda, reinterpret_cast<cuDoubleComplex*>(B), ldb);
}

/*!
 * \brief Wrappers for complex double-precision egblas axdy operation
 * \param n The size of the vector
 * \param alpha The scaling factor alpha
 * \param A The memory of the vector a
 * \param lda The leading dimension of a
 * \param B The memory of the vector b
 * \param ldb The leading dimension of b
 */
inline void axdy(size_t n, etl::complex<double>* alpha, etl::complex<double>* A , size_t lda, etl::complex<double>* B , size_t ldb){
    egblas_zaxdy(n, *reinterpret_cast<cuDoubleComplex*>(alpha), reinterpret_cast<cuDoubleComplex*>(A), lda, reinterpret_cast<cuDoubleComplex*>(B), ldb);
}

#else

static constexpr bool has_zaxdy = false;

#endif

} //end of namespace egblas
} //end of namespace impl
} //end of namespace etl
