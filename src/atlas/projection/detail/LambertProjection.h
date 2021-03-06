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

#include "atlas/projection/detail/ProjectionImpl.h"

namespace atlas {
namespace projection {
namespace detail {

class LambertProjection : public ProjectionImpl {
public:
    // constructor
    LambertProjection( const eckit::Parametrisation& p );

    // destructor
    ~LambertProjection() {}

    // class name
    static std::string static_type() { return "lambert"; }
    virtual std::string type() const override { return static_type(); }

    // projection and inverse projection
    virtual void xy2lonlat( double crd[] ) const override;
    virtual void lonlat2xy( double crd[] ) const override;

    virtual bool strictlyRegional() const override {
        return true;
    }  // lambert projection cannot be used for global grids

    // specification
    virtual Spec spec() const override;

    virtual std::string units() const override { return "meters"; }

    virtual void hash( eckit::Hash& ) const override;

private:
    double lat1_, lat2_;                  // First and second latitude at which the secant cone
                                          // cuts the sphere
    bool is_tangent_;                     // If the first and second latitude are equal, then the
                                          // projection is on a tangent cone
    double lon0_;                         // central longitude
    double radius_;                       // sphere radius
    double n_, inv_n_, F_, rho0_, sign_;  // projection constants

    void setup();
};

}  // namespace detail
}  // namespace projection
}  // namespace atlas
