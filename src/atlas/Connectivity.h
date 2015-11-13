/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Willem Deconinck
/// @date October 2015

#ifndef atlas_Connectivity_H
#define atlas_Connectivity_H

#include "atlas/atlas_config.h"
#include "eckit/memory/Owned.h"
#include "eckit/memory/SharedPtr.h"

namespace atlas {

// Classes defined in this file:
class IrregularConnectivity;
class BlockConnectivity;
class MultiBlockConnectivity;

// --------------------------------------------------------------------------

#ifdef ATLAS_HAVE_FORTRAN
#define FROM_FORTRAN -1
#define TO_FORTRAN +1
#else
#define FROM_FORTRAN
#define TO_FORTRAN
#endif

/// @brief Irregular Connectivity Table
/// @author Willem Deconinck
///
/// Container for irregular connectivity tables. This is e.g. the case
/// for a node-to-X connectivity.
///   connectivity = [
///     1   2   3   4   5   6       # node 1
///     7   8                       # node 2
///     9  10  11  12               # node 3
///    13  14  15                   # node 4
///    16  17  18  19  20  21       # node 5
///   ]
/// There are 2 modes of construction:
///   - It wraps existing raw data
///   - It owns the connectivity data
///
/// In case ATLAS_HAVE_FORTRAN is defined (which is usually the case),
/// the raw data will be stored with base 1 for Fortran interoperability.
/// The operator(row,col) will then do the conversion to base 0.
///
/// In the first mode of construction, the connectivity table cannot be resized.
/// In the second mode of construction, resizing is possible
class IrregularConnectivity : public eckit::Owned
{
public:
//-- Constructors

  /// @brief Construct connectivity table that needs resizing a-posteriori
  /// Data is owned
  IrregularConnectivity();

  /// @brief Construct connectivity table wrapping existing raw data.
  /// No resizing can be performed as data is not owned.
  IrregularConnectivity( idx_t values[], size_t rows, size_t displs[], size_t counts[] );

  ~IrregularConnectivity();

//-- Accessors
  /// @brief Number of rows in the connectivity table
  size_t rows() const { return rows_; }

  /// @brief Number of columns for specified row in the connectivity table
  size_t cols(size_t row) const { return counts_[row]; }

  /// @brief Access to connectivity table elements for given row and column
  /// The returned index has base 0 regardless if ATLAS_HAVE_FORTRAN is defined.
  idx_t operator()(size_t row, size_t col) const;

  /// @brief Access to raw data.
  /// Note that the connectivity base is 1 in case ATLAS_HAVE_FORTRAN is defined.
  const idx_t* data() const { return values_; }
        idx_t* data()       { return values_; }

///-- Modify

  /// @brief Modify row with given values. Values must be given with base 0
  void set(size_t row, const idx_t column_values[]);

  /// @brief Resize connectivity, and add given rows
  /// @note Can only be used when data is owned.
  void add( size_t rows, size_t cols, const idx_t values[], bool fortran_array=false );

  /// @brief Resize connectivity, and copy from a BlockConnectivity
  /// @note Can only be used when data is owned.
  void add( const BlockConnectivity& );


private:
  virtual void on_add() {}

protected:
  bool owns() { return owns_; }
  const size_t *displs() const { return displs_; }
  const size_t *counts() const { return counts_; }

private:
  bool owns_;
  std::vector<idx_t>  owned_values_;
  std::vector<size_t> owned_displs_;
  std::vector<size_t> owned_counts_;

  idx_t  *values_;
  size_t rows_;
  size_t *displs_;
  size_t *counts_;
};

// ----------------------------------------------------------------------------------------------

/// @brief MultiBlockConnectivity Table
/// @author Willem Deconinck
///
/// Container for connectivity tables that are layed out in memory as BlockConnectivities stitched together.
/// This is e.g. the case for a element-node connectivity, with element-types grouped together:
///  connectivity = [
///     # triangles  (block 0)
///         1   2   3
///         4   5   6
///         7   8   9
///        10  11  12
///     # quadrilaterals (block 1)
///        13  14  15  16
///        17  18  19  20
///        21  22  23  24
///  ]
///
/// This class can also be interpreted as a atlas::IrregularConnectivity without distinction between blocks
///
/// There are 2 modes of construction:
///   - It wraps existing raw data
///   - It owns the connectivity data
///
/// In case ATLAS_HAVE_FORTRAN is defined (which is usually the case),
/// the raw data will be stored with base 1 for Fortran interoperability.
/// The operator(row,col) will then do the conversion to base 0.
///
/// In the first mode of construction, the connectivity table cannot be resized.
/// In the second mode of construction, resizing is possible
class MultiBlockConnectivity : public IrregularConnectivity
{
public:
  MultiBlockConnectivity( idx_t values[], size_t rows, size_t displs[], size_t counts[], size_t blocks, size_t block_offset[] );

  MultiBlockConnectivity();
  ~MultiBlockConnectivity();

  size_t blocks() const { return blocks_; }

  const BlockConnectivity& block_connectivity(size_t block_idx) const { return *block_connectivity_[block_idx].get(); }
        BlockConnectivity& block_connectivity(size_t block_idx)       { return *block_connectivity_[block_idx].get(); }

  using IrregularConnectivity::operator();
  idx_t operator()(size_t block, size_t row, size_t col) const;

private:
  virtual void on_add();
  void regenerate_block_connectivity();

private:
  std::vector<size_t> owned_block_offset_;
  size_t blocks_;
  size_t *block_offset_;
  std::vector< eckit::SharedPtr<BlockConnectivity> > block_connectivity_;
};


class BlockConnectivity : public eckit::Owned
{
  friend class IrregularConnectivity;
public:
  BlockConnectivity();
  BlockConnectivity( size_t rows, size_t cols, idx_t values[] );
  idx_t operator()( size_t row, size_t col ) const;
  void set( size_t row, const idx_t column_values[] );
  void add( size_t rows, size_t cols, const idx_t values[], bool fortran_array=false );
  size_t rows() const { return rows_; }
  size_t cols() const { return cols_; }
private:
  bool owns_;
  std::vector<idx_t> owned_values_;

  size_t rows_;
  size_t cols_;
  idx_t *values_;

};

// -----------------------------------------------------------------------------------------------------

inline idx_t IrregularConnectivity::operator()(size_t row, size_t col) const
{
  return (values_+displs_[row])[col] FROM_FORTRAN;
}

inline void IrregularConnectivity::set(size_t row, const idx_t column_values[]) {
  idx_t *col = values_+displs_[row];
  const size_t N = counts_[N];
  for( size_t n=0; n<N; ++n ) {
    col[n] = column_values[n] TO_FORTRAN;
  }
}


inline idx_t MultiBlockConnectivity::operator()(size_t block, size_t row, size_t col) const
{
  return block_connectivity(block)(row,col);
}

// -----------------------------------------------------------------------------------------------------

inline idx_t BlockConnectivity::operator()(size_t row, size_t col) const {
  return (values_+row*cols_)[col] FROM_FORTRAN;
}

inline void BlockConnectivity::set(size_t row, const idx_t column_values[]) {
  idx_t *col = values_+row*cols_;
  for( size_t n=0; n<cols_; ++n ) {
    col[n] = column_values[n] TO_FORTRAN;
  }
}

// ------------------------------------------------------------------------------------------------------

extern "C"
{
}

#undef FROM_FORTRAN
#undef TO_FORTRAN

//------------------------------------------------------------------------------------------------------

} // namespace atlas

#endif
