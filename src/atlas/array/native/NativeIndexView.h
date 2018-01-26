/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file IndexView.h
/// This file contains the IndexView class, a class that allows to wrap any contiguous raw data into
/// a view which is accessible with multiple indices.
/// This view is intended to work with Connectivity Tables storing Fortran Numbering internally
/// All it needs is the strides for each index, and the shape of each index.
/// ATTENTION: The last index is stride 1
///
/// Bounds-checking can be turned ON by defining "ATLAS_INDEXVIEW_BOUNDS_CHECKING"
/// before including this header.
///
/// Example:
/// int[] array = { 1, 2, 3, 4, 5, 6, 7, 8, 9};
/// int[2] strides = { 3, 1 };
/// int[2] shape = { 3, 3 };
/// IndexView<int,2> matrix( array, strides, shape );
/// for( size_t i=0; i<matrix.shape(0); ++i ) {
///   for( size_t j=0; j<matrix.shape(1); ++j ) {
///     matrix(i,j) *= 10;
///   }
/// }
///
/// There is also an easier way to wrap Field and Array classes:
/// IndexView<int,3> fieldview( Field );
/// IndexView<int,2> INDEXVIEW( Array );

#pragma once


#include <iosfwd>
#include "eckit/exception/Exceptions.h"
#include "atlas/array/ArrayUtil.h"
#include "atlas/library/config.h"

//------------------------------------------------------------------------------------------------------

namespace atlas {
namespace array {

//------------------------------------------------------------------------------------------------------

namespace detail {
// FortranIndex:
// Helper class that does +1 and -1 operations on stored values
template< typename Value >
class FortranIndex
{
public:
  enum { BASE = 1 };
public:
  FortranIndex(Value* idx): idx_(idx) {}
  void set(const Value& value) { *(idx_) = value+BASE; }
  Value get() const { return *(idx_)-BASE; }
  void operator=(const Value& value) { set(value); }
  FortranIndex<Value>& operator=(const FortranIndex<Value>& other) { set(other.get()); return *this; }
  FortranIndex<Value>& operator+(const Value& value) { *(idx_)+=value; return *this; }
  FortranIndex<Value>& operator-(const Value& value) { *(idx_)-=value; return *this; }
  FortranIndex<Value>& operator--() { --(*(idx_)); return *this; }
  FortranIndex<Value>& operator++() { ++(*(idx_)); return *this; }

  //implicit conversion
  operator Value() const { return get(); }

private:
  Value* idx_;
};
}


#ifdef ATLAS_HAVE_FORTRAN
#define INDEX_REF Index
#define FROM_FORTRAN -1
#define TO_FORTRAN   +1
#else
#define INDEX_REF *
#define FROM_FORTRAN
#define TO_FORTRAN
#endif

//------------------------------------------------------------------------------------------------------

template< typename Value, int Rank >
class IndexView {
public:

    using value_type = typename remove_const<Value>::type;

#ifdef ATLAS_HAVE_FORTRAN
    typedef detail::FortranIndex<Value> Index;
#else
    typedef Value& Index;
#endif

public:

    IndexView( Value* data, const size_t shape[Rank] );
  
  
// -- Access methods

    template < typename... Idx >
    Index operator()(Idx... idx) {
        check_bounds(idx...);
        return INDEX_REF(&data_[index(idx...)]);
    }

    template < typename... Ints >
    const value_type operator()(Ints... idx) const {
        return data_[index(idx...)] FROM_FORTRAN;
    }

private:

// -- Private methods

    template < typename... Ints >
    constexpr int index_part(int dim, int idx, Ints... next_idx) const {
        return dim < Rank ? idx*strides_[dim] + index_part( dim+1, next_idx..., idx ) : 0 ;
    }

    template < typename... Ints >
    constexpr int index(Ints... idx) const {
      return index_part(0, idx...);
    }

#ifdef ATLAS_ARRAYVIEW_BOUNDS_CHECKING
    template < typename... Ints >
    void check_bounds(Ints... idx) const {
      ASSERT( sizeof...(idx) == Rank );
      return check_bounds_part(0, idx...);
    }
#else
    template < typename... Ints >
    void check_bounds(Ints...) const {}
#endif

    template < typename... Ints >
    void check_bounds_force(Ints... idx) const {
      ASSERT( sizeof...(idx) == Rank );
      return check_bounds_part(0, idx...);
    }

    template < typename... Ints >
    void check_bounds_part(int dim, int idx, Ints... next_idx) const {
        if( dim < Rank ) {
            if( size_t(idx) >= shape_[dim] ) {
                std::ostringstream msg; msg << "ArrayView index " << array_dim(dim) << " out of bounds: " << idx << " >= " << shape_[dim];
                throw eckit::OutOfRange(msg.str(),Here());
            }
            check_bounds_part( dim+1, next_idx..., idx );
        }
    }

    static constexpr char array_dim(size_t dim) {
        return
            dim == 0 ? 'i' :(
            dim == 1 ? 'j' :(
            '*'));
    }

  size_t size() const { return shape_[0]; }

  void dump(std::ostream& os) const;

private:
  Value* data_;
  size_t strides_[Rank];
  size_t shape_[Rank];
};

//------------------------------------------------------------------------------------------------------

} // namespace array
} // namespace atlas