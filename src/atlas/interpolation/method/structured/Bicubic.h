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

#include "atlas/interpolation/method/Method.h"

#include <memory>
#include <string>

#include "eckit/config/Configuration.h"
#include "eckit/memory/NonCopyable.h"

#include "atlas/field/Field.h"
#include "atlas/functionspace/FunctionSpace.h"

namespace atlas {
namespace interpolation {
namespace method {

namespace detail {
class BiCubicKernel;
}

class Bicubic : public Method {
public:
    using Kernel = detail::BiCubicKernel;

public:
    Bicubic( const Config& config );

    virtual ~Bicubic() override {}

    virtual void setup( const Grid& source, const Grid& target ) override;

    virtual void setup( const FunctionSpace& source, const FunctionSpace& target ) override;

    virtual void print( std::ostream& ) const override;

    virtual void execute( const Field& src, Field& tgt ) const override;

    virtual void execute( const FieldSet& src, FieldSet& tgt ) const override;


protected:
    void setup( const FunctionSpace& source );

    virtual const FunctionSpace& source() const override { return source_; }

    virtual const FunctionSpace& target() const override { return target_; }

private:
    template <typename Value, int Rank>
    void execute_impl( const FieldSet& src, FieldSet& tgt ) const;

    template <typename Value, int Rank>
    void execute_impl( const Field& src, Field& tgt ) const;

protected:
    Field target_lonlat_;
    Field target_ghost_;

    FunctionSpace source_;
    FunctionSpace target_;

    bool matrix_free_;

    std::unique_ptr<Kernel> kernel_;
};

}  // namespace method
}  // namespace interpolation
}  // namespace atlas
