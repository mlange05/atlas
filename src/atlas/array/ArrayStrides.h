/*
 * (C) Copyright 2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

#pragma once

#include <stddef.h>
#include <initializer_list>
#include <vector>
#include "atlas/library/config.h"

//------------------------------------------------------------------------------------------------------

namespace atlas {
namespace array {

class ArrayStrides : public std::vector<idx_t> {
private:
    using Base = std::vector<idx_t>;

public:
    ArrayStrides() {}
    ArrayStrides( std::initializer_list<idx_t> list ) : Base( list ) {}
    ArrayStrides( Base&& base ) : Base( std::forward<Base>( base ) ) {}
};

inline ArrayStrides make_strides( std::initializer_list<idx_t> list ) {
    return ArrayStrides( list );
}
inline ArrayStrides make_strides( idx_t size1 ) {
    return ArrayStrides{size1};
}
inline ArrayStrides make_strides( idx_t size1, idx_t size2 ) {
    return ArrayStrides{size1, size2};
}
inline ArrayStrides make_strides( idx_t size1, idx_t size2, idx_t size3 ) {
    return ArrayStrides{size1, size2, size3};
}
inline ArrayStrides make_strides( idx_t size1, idx_t size2, idx_t size3, idx_t size4 ) {
    return ArrayStrides{size1, size2, size3, size4};
}
inline ArrayStrides make_strides( idx_t size1, idx_t size2, idx_t size3, idx_t size4, idx_t size5 ) {
    return ArrayStrides{size1, size2, size3, size4, size5};
}

//------------------------------------------------------------------------------------------------------

}  // namespace array
}  // namespace atlas
