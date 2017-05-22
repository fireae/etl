//=======================================================================
// Copyright (c) 2014-2017 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include "etl/expr/base_temporary_expr.hpp"

//Get the implementations
#include "etl/impl/std/pooling_upsample.hpp"
#include "etl/impl/cudnn/pooling_upsample.hpp"

namespace etl {

/*!
 * \brief A derivative of the 3D max pooling (combine derivative and upsampling for performance)
 * \tparam A The input type
 * \tparam B The output type
 * \tparam C The errors type
 */
template <typename A, typename B, typename C, bool Max>
struct dyn_pool_upsample_3d_expr : base_temporary_expr_tern<dyn_pool_upsample_3d_expr<A, B, C, Max>, A, B, C> {
    using value_type = value_t<A>;                                   ///< The type of value of the expression
    using sub_traits = etl::decay_traits<A>;                         ///< The traits of the first sub type
    using this_type  = dyn_pool_upsample_3d_expr<A, B, C, Max>;           ///< The type of this expression
    using base_type  = base_temporary_expr_tern<this_type, A, B, C>; ///< The base type

    static constexpr auto storage_order = sub_traits::storage_order; ///< The sub storage order

private:
    const size_t c1; ///< The pooling ratio for the first dimension
    const size_t c2; ///< The pooling ratio for the second dimension
    const size_t c3; ///< The pooling ratio for the third dimension

    friend struct etl_traits<dyn_pool_upsample_3d_expr>;

public:
    /*!
     * \brief Construct a new expression
     * \param a The sub expression
     */
    dyn_pool_upsample_3d_expr(A a, B b, C c, size_t c1, size_t c2, size_t c3)
            : base_type(a, b, c), c1(c1), c2(c2), c3(c3) {
        //Nothing else to init
    }

    /*!
     * \brief Validate the transposition dimensions
     * \param a The input matrix
     * \þaram c The output matrix
     */
    template <typename R>
    void check(const A& a, const B& b, const C& c, const R& result) const {
        cpp_unused(a);
        cpp_unused(b);
        cpp_unused(c);
        cpp_unused(result);

        static constexpr size_t D = etl::decay_traits<A>::dimensions();

        static_assert(etl::decay_traits<B>::dimensions() == D, "Invalid dimensions in max_pool_upsampl_3d");
        static_assert(etl::decay_traits<C>::dimensions() == D, "Invalid dimensions in max_pool_upsampl_3d");
        static_assert(etl::decay_traits<R>::dimensions() == D, "Invalid dimensions in max_pool_upsampl_3d");

        cpp_assert(etl::size(result) == etl::size(a), "max_pool_upsample_3d:A and R must have the same size");
        cpp_assert(etl::size(b) == etl::size(c), "max_pool_upsample_3d:B and C must have the same size");

        cpp_assert(etl::dim<D - 3>(a) == c1 * etl::dim<D - 3>(b), "Invalid pooling dimensions for max_pool_upsample_2d");
        cpp_assert(etl::dim<D - 2>(a) == c2 * etl::dim<D - 2>(b), "Invalid pooling dimensions for max_pool_upsample_2d");
        cpp_assert(etl::dim<D - 1>(a) == c3 * etl::dim<D - 1>(b), "Invalid pooling dimensions for max_pool_upsample_2d");
    }

    // Assignment functions

    /*!
     * \brief Select the pool implementation for an expression of type ABC->R
     *
     * This does not consider the local context
     *
     * \return The implementation to use
     */
    template <typename R>
    static cpp14_constexpr etl::pool_impl select_default_impl() {
        if (cudnn_enabled && all_floating<A, B, C, R>::value) {
            return etl::pool_impl::CUDNN;
        }

        return etl::pool_impl::STD;
    }

    /*!
     * \brief Select the pool implementation for an expression of type ABC->R
     * \return The implementation to use
     */
    template <typename R>
    static etl::pool_impl select_impl() {
        if (local_context().sum_selector.forced) {
            auto forced = local_context().pool_selector.impl;

            switch (forced) {
                // CUDNN cannot always be used
                case pool_impl::CUDNN:
                    if (!cudnn_enabled || !all_floating<A, B, C, R>::value) {                                                            //COVERAGE_EXCLUDE_LINE
                        std::cerr << "Forced selection to CUDNN pool implementation, but not possible for this expression" << std::endl; //COVERAGE_EXCLUDE_LINE
                        return select_default_impl<R>();                                                                                 //COVERAGE_EXCLUDE_LINE
                    }                                                                                                                    //COVERAGE_EXCLUDE_LINE

                    return forced;

                //In other cases, simply use the forced impl
                default:
                    return forced;
            }
        }

        return select_default_impl<R>();
    }

    /*!
     * \brief Assign to a matrix of the same storage order
     * \param result The expression to which assign
     */
    template <typename R>
    void assign_to(R&& result) const {
        static_assert(all_etl_expr<A, B, C, R>::value, "Max Pool Derivative only supported for ETL expressions");

        auto& a = this->a();
        auto& b = this->b();
        auto& c = this->c();

        check(a, b, c, result);

        auto impl = select_impl<R>();

        if /*constexpr*/ (Max) {
            if (impl == pool_impl::STD) {
                impl::standard::max_pool_upsample_3d::apply(
                    make_temporary(a),
                    make_temporary(b),
                    make_temporary(c),
                    result,
                    c1, c2, c3);
            } else if (impl == pool_impl::CUDNN) {
                impl::cudnn::max_pool_upsample_3d::apply(
                    make_temporary(a),
                    make_temporary(b),
                    make_temporary(c),
                    result,
                    c1, c2, c3);
            } else {
                cpp_unreachable("Invalid pool implementation");
            }
        } else {
            if (impl == pool_impl::STD) {
                impl::standard::avg_pool_upsample_3d::apply(
                    make_temporary(a),
                    make_temporary(b),
                    make_temporary(c),
                    result,
                    c1, c2, c3);
            } else if (impl == pool_impl::CUDNN) {
                impl::cudnn::avg_pool_upsample_3d::apply(
                    make_temporary(a),
                    make_temporary(b),
                    make_temporary(c),
                    result,
                    c1, c2, c3);
            } else {
                cpp_unreachable("Invalid pool implementation");
            }
        }
    }

    /*!
     * \brief Add to the given left-hand-side expression
     * \param lhs The expression to which assign
     */
    template <typename L>
    void assign_add_to(L&& lhs) const {
        std_add_evaluate(*this, lhs);
    }

    /*!
     * \brief Sub from the given left-hand-side expression
     * \param lhs The expression to which assign
     */
    template <typename L>
    void assign_sub_to(L&& lhs) const {
        std_sub_evaluate(*this, lhs);
    }

    /*!
     * \brief Multiply the given left-hand-side expression
     * \param lhs The expression to which assign
     */
    template <typename L>
    void assign_mul_to(L&& lhs) const {
        std_mul_evaluate(*this, lhs);
    }

    /*!
     * \brief Divide the given left-hand-side expression
     * \param lhs The expression to which assign
     */
    template <typename L>
    void assign_div_to(L&& lhs) const {
        std_div_evaluate(*this, lhs);
    }

    /*!
     * \brief Modulo the given left-hand-side expression
     * \param lhs The expression to which assign
     */
    template <typename L>
    void assign_mod_to(L&& lhs) const {
        std_mod_evaluate(*this, lhs);
    }

    /*!
     * \brief Print a representation of the expression on the given stream
     * \param os The output stream
     * \param expr The expression to print
     * \return the output stream
     */
    friend std::ostream& operator<<(std::ostream& os, const dyn_pool_upsample_3d_expr& expr) {
        return os << "max_pool_upsample3(" << expr._a << ", " << expr._b << ", " << expr._c << ")";
    }
};

/*!
 * \brief Traits for a pooling usample expression
 * \tparam A The pooling usample sub type
 */
template <typename A, typename B, typename C, bool Max>
struct etl_traits<etl::dyn_pool_upsample_3d_expr<A, B, C, Max>> {
    using expr_t     = etl::dyn_pool_upsample_3d_expr<A, B, C, Max>; ///< The expression type
    using sub_expr_t = std::decay_t<A>;                         ///< The sub expression type
    using sub_traits = etl_traits<sub_expr_t>;                  ///< The sub traits
    using value_type = value_t<A>;                              ///< The value type of the expression

    static constexpr bool is_etl         = true;                      ///< Indicates if the type is an ETL expression
    static constexpr bool is_transformer = false;                     ///< Indicates if the type is a transformer
    static constexpr bool is_view        = false;                     ///< Indicates if the type is a view
    static constexpr bool is_magic_view  = false;                     ///< Indicates if the type is a magic view
    static constexpr bool is_fast        = false;                     ///< Indicates if the expression is fast
    static constexpr bool is_linear      = true;                      ///< Indicates if the expression is linear
    static constexpr bool is_thread_safe = true;                      ///< Indicates if the expression is thread safe
    static constexpr bool is_value       = false;                     ///< Indicates if the expression is of value type
    static constexpr bool is_direct      = true;                      ///< Indicates if the expression has direct memory access
    static constexpr bool is_generator   = false;                     ///< Indicates if the expression is a generator
    static constexpr bool is_padded      = false;                     ///< Indicates if the expression is padded
    static constexpr bool is_aligned     = true;                      ///< Indicates if the expression is padded
    static constexpr bool is_gpu         = false;                     ///< Indicates if the expression can be done on GPU
    static constexpr bool is_temporary   = true;                      ///< Indicates if the expression needs a evaluator visitor
    static constexpr order storage_order = sub_traits::storage_order; ///< The expression's storage order

    /*!
     * \brief Indicates if the expression is vectorizable using the
     * given vector mode
     * \tparam V The vector mode
     */
    template <vector_mode_t V>
    using vectorizable = std::true_type;

    /*!
     * \brief Returns the dth dimension of the expression
     * \param e The sub expression
     * \param d The dimension to get
     * \return the dth dimension of the expression
     */
    static size_t dim(const expr_t& e, size_t d) {
        return etl::dim(e.a(), d);
    }

    /*!
     * \brief Returns the size of the expression
     * \param e The sub expression
     * \return the size of the expression
     */
    static size_t size(const expr_t& e) {
        return etl::size(e.a());
    }

    /*!
     * \brief Returns the number of dimensions of the expression
     * \return the number of dimensions of the expression
     */
    static constexpr size_t dimensions() {
        return sub_traits::dimensions();
    }
};

/*!
 * \brief Derivative of the 3D Max Pooling of the given matrix expression and upsampling.
 * \param input The input
 * \param output The output
 * \param c1 The first pooling ratio
 * \param c2 The second pooling ratio
 * \param c3 The third pooling ratio
 * \return A expression representing the Derivative of 3D Max Pooling of the input expression.
 */
template <typename A, typename B, typename C>
dyn_pool_upsample_3d_expr<detail::build_type<A>, detail::build_type<B>, detail::build_type<C>, true>
max_pool_upsample_3d(A&& input, B&& output, C&& errors, size_t c1, size_t c2, size_t c3) {
    return {input, output, errors, c1, c2, c3};
}

/*!
 * \brief Derivative of the 3D Average Pooling of the given matrix expression and upsampling.
 * \param input The input
 * \param output The output
 * \param c1 The first pooling ratio
 * \param c2 The second pooling ratio
 * \param c3 The third pooling ratio
 * \return A expression representing the Derivative of 3D Average Pooling of the input expression.
 */
template <typename A, typename B, typename C>
dyn_pool_upsample_3d_expr<detail::build_type<A>, detail::build_type<B>, detail::build_type<C>, false>
avg_pool_upsample_3d(A&& input, B&& output, C&& errors, size_t c1, size_t c2, size_t c3) {
    return {input, output, errors, c1, c2, c3};
}

} //end of namespace etl
