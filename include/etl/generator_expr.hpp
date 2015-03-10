//=======================================================================
// Copyright (c) 2014-2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef ETL_GENERATOR_EXPR_HPP
#define ETL_GENERATOR_EXPR_HPP

#include <iostream> //For stream support

namespace etl {

template <typename Generator>
class generator_expr final {
private:
    mutable Generator generator;

public:
    using value_type = typename Generator::value_type;

    template<typename... Args>
    generator_expr(Args... args) : generator(std::forward<Args>(args)...) {}

    generator_expr(const generator_expr& e) : generator(e.generator) {
        //Nothing else to init
    }

    generator_expr(generator_expr&& e) : generator(std::move(e.generator)) {
        //Nothing else to init
    }

    //Expression are invariant
    generator_expr& operator=(const generator_expr&) = delete;
    generator_expr& operator=(generator_expr&&) = delete;

    //Apply the expression

    value_type operator[](std::size_t) const {
        return generator();
    }

    value_type operator()() const {
        return generator();
    }

    const Generator& get_generator() const {
        return generator;
    }
};

template <typename Generator>
std::ostream& operator<<(std::ostream& os, const generator_expr<Generator>& expr){
    return os << expr.get_generator();
}

} //end of namespace etl

#endif
