//=======================================================================
// Copyright (c) 2014 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef ETL_old_matrix_HPP
#define ETL_old_matrix_HPP

#include <vector> //To store the data
#include <array>  //To store the dimensions
#include <tuple>  //For TMP stuff

#include "assert.hpp"
#include "tmp.hpp"
#include "fast_op.hpp"
#include "fast_expr.hpp"

namespace etl {

//TODO Try to pass the index_sequence as a type, not a parameter

enum class init_flag_t { DUMMY };
constexpr const init_flag_t init_flag = init_flag_t::DUMMY;

template<typename... V>
struct values_t {
    const std::tuple<V...> values;
    values_t(V... v) : values(v...) {};

    template<typename T, std::size_t... I>
    std::vector<T> list_sub(const std::index_sequence<I...>& /*i*/) const {
        return {static_cast<T>(std::get<I>(values))...};
    }

    template<typename T>
    std::vector<T> list() const {
        return list_sub<T>(std::make_index_sequence<sizeof...(V)>());
    }
};

template<typename... V>
values_t<V...> values(V... v){
    return {v...};
}

namespace dyn_detail {

template<typename... S>
struct is_init_constructor : std::integral_constant<bool, false> {};

template<typename S1, typename S2, typename S3, typename... S>
struct is_init_constructor<S1, S2, S3, S...> :
    std::integral_constant<bool, std::is_same<init_flag_t, typename nth_type<1+sizeof...(S), S1, S2, S3, S...>::type>::value> {};

template<typename... S>
struct is_initializer_list_constructor : std::integral_constant<bool, false> {};

template<typename S1, typename S2, typename... S>
struct is_initializer_list_constructor<S1, S2, S...> :
    std::integral_constant<bool, is_specialization_of<std::initializer_list, typename last_type<S1, S2, S...>::type>::value> {};

inline std::size_t size(std::size_t first){
    return first;
}

template<typename... T>
inline std::size_t size(std::size_t first, T... args){
    return first * size(args...);
}

template<std::size_t... I, typename... T>
inline std::size_t size(const std::index_sequence<I...>& /*i*/, const T&... args){
    return size((nth_value<I>(args...))...);
}

template<std::size_t... I, typename... T>
inline std::array<std::size_t, sizeof...(I)> sizes(const std::index_sequence<I...>& /*i*/, const T&... args){
    return {{static_cast<std::size_t>(nth_value<I>(args...))...}};
}

} // end of namespace dyn_detail

template<typename T, std::size_t D = 2>
struct dyn_matrix {
    static_assert(D > 0, "A matrix must have a least 1 dimension");

public:
    static constexpr const std::size_t n_dimensions = D;

    using                value_type = T;
    using              storage_impl = std::vector<value_type>;
    using    dimension_storage_impl = std::array<std::size_t, n_dimensions>;
    using                  iterator = typename storage_impl::iterator;
    using            const_iterator = typename storage_impl::const_iterator;

private:
    const std::size_t _size;
    storage_impl _data;
    dimension_storage_impl _dimensions;

public:
    //{{{ Construction

    dyn_matrix(const dyn_matrix& rhs) : _size(rhs._size), _data(rhs._data), _dimensions(rhs._dimensions) {
        //Nothing to init
    }

    //Sizes followed by an initializer list
    dyn_matrix(std::initializer_list<value_type> list) :
            _size(list.size()),
            _data(list),
            _dimensions({{list.size()}}) {
        static_assert(n_dimensions == 1, "This constructor can only be used for 1D matrix");
        //Nothing to init
    }

    //Normal constructor with only sizes
    template<typename... S, enable_if_u<
        and_u<
            (sizeof...(S) == D),
            all_convertible_to<std::size_t, S...>::value,
            is_homogeneous<typename first_type<S...>::type, S...>::value
        >::value> = detail::dummy>
    dyn_matrix(S... sizes) :
            _size(dyn_detail::size(sizes...)),
            _data(_size),
            _dimensions{{static_cast<std::size_t>(sizes)...}} {
        //Nothing to init
    }

    //Sizes followed by an initializer list
    template<typename... S, enable_if_u<dyn_detail::is_initializer_list_constructor<S...>::value> = detail::dummy>
    dyn_matrix(S... sizes) :
            _size(dyn_detail::size(std::make_index_sequence<(sizeof...(S)-1)>(), sizes...)),
            _data(last_value(sizes...)),
            _dimensions(dyn_detail::sizes(std::make_index_sequence<(sizeof...(S)-1)>(), sizes...)) {
        static_assert(sizeof...(S) == D + 1, "Invalid number of dimensions");
    }

    //Sizes followed by a values_t
    template<typename S1, typename... S, enable_if_u<
        and_u<
            (sizeof...(S) == D),
            is_specialization_of<values_t, typename last_type<S1, S...>::type>::value
        >::value> = detail::dummy>
    dyn_matrix(S1 s1, S... sizes) :
            _size(dyn_detail::size(std::make_index_sequence<(sizeof...(S))>(), s1, sizes...)),
            _data(last_value(s1, sizes...).template list<value_type>()),
            _dimensions(dyn_detail::sizes(std::make_index_sequence<(sizeof...(S))>(), s1, sizes...)) {
        //Nothing to init
    }

    //Sizes followed by a value
    template<typename S1, typename... S, enable_if_u<
        and_u<
            (sizeof...(S) == D),
            std::is_convertible<std::size_t, typename first_type<S1, S...>::type>::value,   //The first type must be convertible to size_t
            is_sub_homogeneous<S1, S...>::value,                                            //The first N-1 types must homegeneous
            std::is_same<value_type, typename last_type<S1, S...>::type>::value             //The last type must be exactly value_type
        >::value> = detail::dummy>
    dyn_matrix(S1 s1, S... sizes) :
            _size(dyn_detail::size(std::make_index_sequence<(sizeof...(S))>(), s1, sizes...)),
            _data(_size, last_value(s1, sizes...)),
            _dimensions(dyn_detail::sizes(std::make_index_sequence<(sizeof...(S))>(), s1, sizes...)) {
        //Nothing to init
    }

    //Sizes followed by an init flag followed by the value
    template<typename... S, enable_if_u<dyn_detail::is_init_constructor<S...>::value> = detail::dummy>
    dyn_matrix(S... sizes) :
            _size(dyn_detail::size(std::make_index_sequence<(sizeof...(S)-2)>(), sizes...)),
            _data(_size, last_value(sizes...)),
            _dimensions(dyn_detail::sizes(std::make_index_sequence<(sizeof...(S)-2)>(), sizes...)) {
        static_assert(sizeof...(S) == D + 2, "Invalid number of dimensions");

        //Nothing to init
    }

    template<typename LE, typename Op, typename RE>
    explicit dyn_matrix(const binary_expr<value_type, LE, Op, RE>& e) :
            _size(etl::size(e)),
            _data(_size) {

        for(std::size_t d = 0; d < etl::dimensions(e); ++d){
            _dimensions[d] = etl::dim(e, d);
        }

        for(std::size_t i = 0; i < size(); ++i){
            _data[i] = e[i];
        }
    }

    template<typename E, typename Op>
    explicit dyn_matrix(const unary_expr<value_type, E, Op>& e) :
            _size(etl::size(e)),
            _data(_size) {

        for(std::size_t d = 0; d < etl::dimensions(e); ++d){
            _dimensions[d] = etl::dim(e, d);
        }

        for(std::size_t i = 0; i < size(); ++i){
            _data[i] = e[i];
        }
    }

    template<typename E, enable_if_u<etl_traits<transform_expr<value_type, E>>::dimensions() == 1> = detail::dummy>
    explicit dyn_matrix(const transform_expr<value_type, E>& e) :
            _size(etl::size(e)),
            _data(_size) {

        for(std::size_t d = 0; d < etl::dimensions(e); ++d){
            _dimensions[d] = etl::dim(e, d);
        }

        for(std::size_t i = 0; i < rows(); ++i){
            _data[i] = e(i);
        }
    }

    template<typename E, enable_if_u<etl_traits<transform_expr<value_type, E>>::dimensions() == 2> = detail::dummy>
    explicit dyn_matrix(const transform_expr<value_type, E>& e) :
            _size(etl::size(e)),
            _data(_size) {

        for(std::size_t d = 0; d < etl::dimensions(e); ++d){
            _dimensions[d] = etl::dim(e, d);
        }

        for(std::size_t i = 0; i < rows(); ++i){
            for(std::size_t j = 0; j < columns(); ++j){
                _data[i * columns() + j] = e(i,j);
            }
        }
    }

    template<typename Container, enable_if_u<std::is_convertible<typename Container::value_type, value_type>::value> = detail::dummy>
    dyn_matrix(const Container& vec) : _size(vec.size()), _data(_size) {
        static_assert(D == 1, "Only 1D matrix can be constructed from containers");

        for(std::size_t i = 0; i < size(); ++i){
            _data[i] = vec[i];
        }
    }

    //Default move
    dyn_matrix(dyn_matrix&& rhs) = default;

    //}}}

    //{{{ Assignment

    //Copy assignment operator

    dyn_matrix& operator=(const dyn_matrix& rhs){
        ensure_same_size(*this, rhs);

        for(std::size_t i = 0; i < size(); ++i){
            _data[i] = rhs[i];
        }

        return *this;
    }

    //Allow copy from other containers

    template<typename Container, enable_if_u<std::is_convertible<typename Container::value_type, value_type>::value> = detail::dummy>
    dyn_matrix& operator=(const Container& vec){
        etl_assert(vec.size() == size(), "Cannot copy from a vector of different size");

        for(std::size_t i = 0; i < size(); ++i){
            _data[i] = vec[i];
        }

        return *this;
    }

    //Construct from expression

    template<typename LE, typename Op, typename RE>
    dyn_matrix& operator=(binary_expr<value_type, LE, Op, RE>&& e){
        ensure_same_size(*this, e);

        for(std::size_t i = 0; i < size(); ++i){
            _data[i] = e[i];
        }

        return *this;
    }

    template<typename E, typename Op>
    dyn_matrix& operator=(unary_expr<value_type, E, Op>&& e){
        ensure_same_size(*this, e);

        for(std::size_t i = 0; i < size(); ++i){
            _data[i] = e[i];
        }

        return *this;
    }

    template<typename E, enable_if_u<etl_traits<transform_expr<value_type, E>>::dimensions() == 1> = detail::dummy>
    dyn_matrix& operator=(transform_expr<value_type, E>&& e){
        ensure_same_size(*this, e);

        for(std::size_t i = 0; i < rows(); ++i){
            _data[index(i)] = e(i);
        }

        return *this;
    }

    template<typename E, enable_if_u<etl_traits<transform_expr<value_type, E>>::dimensions() == 2> = detail::dummy>
    dyn_matrix& operator=(transform_expr<value_type, E>&& e){
        ensure_same_size(*this, e);

        for(std::size_t i = 0; i < rows(); ++i){
            for(std::size_t j = 0; j < columns(); ++j){
                _data[index(i,j)] = e(i,j);
            }
        }

        return *this;
    }

    //Set the same value to each element of the matrix
    dyn_matrix& operator=(const value_type& value){
        std::fill(_data.begin(), _data.end(), value);

        return *this;
    }

    //Default move
    dyn_matrix& operator=(dyn_matrix&& rhs) = default;

    //}}}

    //{{{ Accessors

    size_t size() const {
        return _size;
    }

    size_t rows() const {
        return _dimensions[0];
    }

    size_t columns() const {
        static_assert(n_dimensions > 1, "columns() only valid for 2D+ matrices");
        return _dimensions[1];
    }

    static constexpr size_t dimensions(){
        return n_dimensions;
    }

    size_t dim(std::size_t d) const {
        etl_assert(d < n_dimensions, "Invalid dimension");

        return _dimensions[d];
    }

    value_type& operator()(size_t i){
        etl_assert(i < dim(0), "Out of bounds");
        etl_assert(n_dimensions == 1, "Invalid number of parameters");

        return _data[i];
    }

    const value_type& operator()(size_t i) const {
        etl_assert(i < dim(0), "Out of bounds");
        static_assert(n_dimensions == 1, "Invalid number of parameters");

        return _data[i];
    }

    value_type& operator()(size_t i, size_t j){
        etl_assert(i < dim(0), "Out of bounds");
        etl_assert(j < dim(1), "Out of bounds");
        static_assert(n_dimensions == 2, "Invalid number of parameters");

        return _data[i * dim(1) + j];
    }

    const value_type& operator()(size_t i, size_t j) const {
        etl_assert(i < dim(0), "Out of bounds");
        etl_assert(j < dim(1), "Out of bounds");
        static_assert(dimensions() == 2, "Invalid number of parameters");

        return _data[i * dim(1) + j];
    }

    template<typename... S, enable_if_u<(sizeof...(S) > 0)> = detail::dummy>
    std::size_t index(S... sizes) const {
        //Note: Version with sizes moved to a std::array and accessed with
        //standard loop may be faster, but need some stack space (relevant ?)

        auto subsize = size();
        std::size_t index = 0;
        std::size_t i = 0;

        for_each_in(
            [&subsize, &index, &i, this](std::size_t s){
                subsize /= dim(i++);
                index += subsize * s;
            }, sizes...);

        return index;
    }

    template<typename... S, enable_if_u<
        and_u<
            (sizeof...(S) > 2),
            all_convertible_to<std::size_t, S...>::value
        >::value> = detail::dummy>
    const value_type& operator()(S... sizes) const {
        static_assert(sizeof...(S) == n_dimensions, "Invalid number of parameters");

        return _data[index(sizes...)];
    }

    template<typename... S, enable_if_u<
        and_u<
            (sizeof...(S) > 2),
            all_convertible_to<std::size_t, S...>::value
        >::value> = detail::dummy>
    value_type& operator()(S... sizes){
        static_assert(sizeof...(S) == n_dimensions, "Invalid number of parameters");

        return _data[index(sizes...)];
    }

    const value_type& operator[](size_t i) const {
        etl_assert(i < size(), "Out of bounds");

        return _data[i];
    }

    value_type& operator[](size_t i){
        etl_assert(i < size(), "Out of bounds");

        return _data[i];
    }

    const_iterator begin() const {
        return _data.begin();
    }

    const_iterator end() const {
        return _data.end();
    }

    iterator begin(){
        return _data.begin();
    }

    iterator end(){
        return _data.end();
    }

    //}}}
};

} //end of namespace etl

#endif
