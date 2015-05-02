//=======================================================================
// Copyright (c) 2014-2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef ETL_BINARY_OP_HPP
#define ETL_BINARY_OP_HPP

#include <random>
#include <functional>
#include <ctime>

#include "math.hpp"

namespace etl {

using random_engine = std::mt19937_64;

template<typename T>
struct simple_operator : std::true_type {};

template<typename T>
struct plus_binary_op {
    static constexpr const bool vectorizable = true;

    static constexpr T apply(const T& lhs, const T& rhs) noexcept {
        return lhs + rhs;
    }

    static std::string desc() noexcept {
        return "+";
    }
};

template<typename T>
struct minus_binary_op {
    static constexpr const bool vectorizable = true;

    static constexpr T apply(const T& lhs, const T& rhs) noexcept {
        return lhs - rhs;
    }

    static std::string desc() noexcept {
        return "-";
    }
};

template<typename T>
struct mul_binary_op {
    static constexpr const bool vectorizable = true;

    static constexpr T apply(const T& lhs, const T& rhs) noexcept {
        return lhs * rhs;
    }

    static std::string desc() noexcept {
        return "*";
    }
};

template<typename T>
struct div_binary_op {
    static constexpr const bool vectorizable = true;

    static constexpr T apply(const T& lhs, const T& rhs) noexcept {
        return lhs / rhs;
    }

    static std::string desc() noexcept {
        return "/";
    }
};

template<typename T>
struct mod_binary_op {
    static constexpr const bool vectorizable = false;

    static constexpr T apply(const T& lhs, const T& rhs) noexcept {
        return lhs % rhs;
    }

    static std::string desc() noexcept {
        return "%";
    }
};

template<typename T, typename E>
struct ranged_noise_binary_op {
    static constexpr const bool vectorizable = false;

    static T apply(const T& x, E value){
        static random_engine rand_engine(std::time(nullptr));
        static std::normal_distribution<double> normal_distribution(0.0, 1.0);
        static auto noise = std::bind(normal_distribution, rand_engine);

        if(x == 0.0 || x == value){
            return x;
        } else {
            return x + noise();
        }
    }

    static std::string desc() noexcept {
        return "ranged_noise";
    }
};

template<typename T, typename E>
struct max_binary_op {
    static constexpr const bool vectorizable = false;

    static constexpr T apply(const T& x, E value) noexcept {
        return std::max(x, value);
    }

    static std::string desc() noexcept {
        return "max";
    }
};

template<typename T, typename E>
struct min_binary_op {
    static constexpr const bool vectorizable = false;

    static constexpr T apply(const T& x, E value) noexcept {
        return std::min(x, value);
    }

    static std::string desc() noexcept {
        return "min";
    }
};

template<typename T, typename E>
struct pow_binary_op {
    static constexpr const bool vectorizable = false;

    static constexpr T apply(const T& x, E value) noexcept {
        return std::pow(x, value);
    }

    static std::string desc() noexcept {
        return "pow";
    }
};

template<typename T, typename E>
struct one_if_binary_op {
    static constexpr const bool vectorizable = false;

    static constexpr T apply(const T& x, E value) noexcept {
        return 1.0 ? x == value : 0.0;
    }

    static std::string desc() noexcept {
        return "one_if";
    }
};

template<typename T, typename E>
struct simple_operator<ranged_noise_binary_op<T, E>> : std::false_type {};

template<typename T, typename E>
struct simple_operator<max_binary_op<T, E>> : std::false_type {};

template<typename T, typename E>
struct simple_operator<min_binary_op<T, E>> : std::false_type {};

template<typename T, typename E>
struct simple_operator<pow_binary_op<T, E>> : std::false_type {};

template<typename T, typename E>
struct simple_operator<one_if_binary_op<T, E>> : std::false_type {};


} //end of namespace etl

#endif
