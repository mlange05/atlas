/*
 * (C) Copyright 2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

#include "atlas/grid/detail/partitioner/MatchingMeshPartitionerLonLatPolygon.h"

#include <vector>

#include "eckit/config/Resource.h"
#include "eckit/log/ProgressTimer.h"

#include "atlas/grid/Grid.h"
#include "atlas/mesh/Nodes.h"
#include "atlas/parallel/mpi/mpi.h"
#include "atlas/runtime/Log.h"
#include "atlas/util/LonLatPolygon.h"

namespace atlas {
namespace grid {
namespace detail {
namespace partitioner {

namespace {
PartitionerBuilder<MatchingMeshPartitionerLonLatPolygon> __builder( "lonlat-polygon" );
}

void MatchingMeshPartitionerLonLatPolygon::partition( const Grid& grid, int partitioning[] ) const {
    const eckit::mpi::Comm& comm = atlas::mpi::comm();
    const int mpi_rank           = int( comm.rank() );
    const int mpi_size           = int( comm.size() );

    ATLAS_TRACE( "MatchingMeshPartitionerLonLatPolygon::partition" );

    ASSERT( grid.domain().global() );

    Log::debug() << "MatchingMeshPartitionerLonLatPolygon::partition" << std::endl;

    // FIXME: THIS IS A HACK! the coordinates include North/South Pole (first/last
    // partitions only)
    bool includesNorthPole = ( mpi_rank == 0 );
    bool includesSouthPole = ( mpi_rank == mpi_size - 1 );

    const util::LonLatPolygon poly( prePartitionedMesh_.polygon( 0 ), prePartitionedMesh_.nodes().lonlat() );

    {
        eckit::ProgressTimer timer( "Partitioning", grid.size(), "point", double( 10 ), atlas::Log::trace() );
        size_t i = 0;

        for ( const PointXY Pxy : grid.xy() ) {
            ++timer;
            const PointLonLat P  = grid.projection().lonlat( Pxy );
            const bool atThePole = ( includesNorthPole && P.lat() >= poly.coordinatesMax().lat() ) ||
                                   ( includesSouthPole && P.lat() < poly.coordinatesMin().lat() );

            partitioning[i++] = atThePole || poly.contains( P ) ? mpi_rank : -1;
        }
    }

    // Synchronize partitioning, do a sanity check
    comm.allReduceInPlace( partitioning, grid.size(), eckit::mpi::Operation::MAX );
    const int min = *std::min_element( partitioning, partitioning + grid.size() );
    if ( min < 0 ) {
        throw eckit::SeriousBug(
            "Could not find partition for target node (source "
            "mesh does not contain all target grid points)",
            Here() );
    }
}

}  // namespace partitioner
}  // namespace detail
}  // namespace grid
}  // namespace atlas
