/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "atlas/grid/global/lonlat/LonLat.h"


namespace atlas {
namespace grid {
namespace global {
namespace lonlat {


std::string LonLat::grid_type_str() {
    return "reduced_lonlat";
}


std::string LonLat::className() {
    return "atlas.grid.global.lonlat.LonLat";
}


void LonLat::setup(const size_t N, const Domain& dom) {
    const size_t
    nlon = 4*N,
    nlat = 2*N;
    LonLat::setup(nlon,nlat,dom);
}


void LonLat::setup(const size_t nlon, const size_t nlat, const Domain& dom) {
#if 0
    // set domain (optional)
    Domain dom = domain();
    std::vector<double> domv;
    if (p.get("domain",domv) && domv.size()==4) {
        dom = Domain(domv[0],domv[1],domv[2],domv[3]);
    } else if (p.has("domain")) {
        throw eckit::BadParameter("Params domain of wrong size, expected #4");
    }
#endif
    ASSERT(nlat>0);

    const bool
    isShiftedLon = shift_.lon(),
    isShiftedLat = shift_.lat();

    const double
    lon_inc = (dom.east()  - dom.west() )/static_cast<double>(nlon),
    lat_inc = (dom.north() - dom.south())/static_cast<double>(isShiftedLat? nlat : nlat-1);

    std::vector<double> lonmin(nlat,dom.west() + (isShiftedLon? lon_inc*0.5 : 0.) );

    std::vector<double> lats(nlat);
    lats[0] = dom.north() - (isShiftedLat? lat_inc*0.5 : 0);
    for (size_t i=1; i<nlat; ++i) {
        lats[i] = lats[0] - lat_inc*static_cast<double>(i);
    }

    std::vector<long> pl(nlat,nlon);

    Structured::setup(nlat, lats.data(), pl.data(), lonmin.data());

    Structured::N_ = !dom.isGlobal()? 0
                     :  isShiftedLat && ( nlat   %2==0) && (nlon==2* nlat   )? size_t( nlat   /2)
                     : !isShiftedLat && ((nlat-1)%2==0) && (nlon==2*(nlat-1))? size_t((nlat-1)/2)
                     : 0;

    set_typeinfo();
}


void LonLat::setup(const long pl[], const size_t nlat, const Domain& dom) {

    ASSERT(nlat>0);
    const bool
    isShiftedLon = shift_.lon(),
    isShiftedLat = shift_.lat();

    const double lat_inc = (dom.north() - dom.south())/static_cast<double>(isShiftedLat? nlat : nlat-1);

    std::vector<double> lonmin(nlat);
    for (size_t i=0; i<nlat; ++i) {
        const double lon_inc = (dom.east() - dom.west() )/static_cast<double>(pl[i]);
        lonmin[i] = dom.west() + (isShiftedLon? lon_inc*0.5 : 0.);
    }

    std::vector<double> lats(nlat);
    lats[0] = dom.north() - (isShiftedLat? lat_inc*0.5 : 0);
    for (size_t i=1; i<nlat; ++i) {
        lats[i] = lats[0] - lat_inc*static_cast<double>(i);
    }

    Structured::setup(nlat, lats.data(), pl, lonmin.data());
    Structured::N_ = 0;

    set_typeinfo();
}


LonLat::LonLat(const Shift& shift, const Domain& dom) :
    Structured(dom),
    shift_(shift) {
}


} // namespace lonlat
} // namespace global
} // namespace grid
} // namespace atlas
