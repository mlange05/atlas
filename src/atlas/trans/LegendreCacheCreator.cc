/*
 * (C) Copyright 2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"

#include "atlas/grid/Grid.h"
#include "atlas/library/defines.h"
#include "atlas/runtime/Log.h"
#include "atlas/trans/LegendreCacheCreator.h"

// For factory registration only:
#if ATLAS_HAVE_TRANS
#include "atlas/trans/ifs/LegendreCacheCreatorIFS.h"
#endif
#include "atlas/trans/local/LegendreCacheCreatorLocal.h"

namespace atlas {
namespace trans {

LegendreCacheCreatorImpl::~LegendreCacheCreatorImpl() {}

namespace {

static eckit::Mutex* local_mutex                              = 0;
static std::map<std::string, LegendreCacheCreatorFactory*>* m = 0;
static pthread_once_t once                                    = PTHREAD_ONCE_INIT;

static void init() {
    local_mutex = new eckit::Mutex();
    m           = new std::map<std::string, LegendreCacheCreatorFactory*>();
}

template <typename T>
void load_builder() {
    LegendreCacheCreatorBuilder<T>( "tmp" );
}

struct force_link {
    force_link() {
#if ATLAS_HAVE_TRANS
        load_builder<LegendreCacheCreatorIFS>();
#endif
        load_builder<LegendreCacheCreatorLocal>();
    }
};

LegendreCacheCreatorFactory& factory( const std::string& name ) {
    std::map<std::string, LegendreCacheCreatorFactory*>::const_iterator j = m->find( name );
    if ( j == m->end() ) {
        Log::error() << "No LegendreCacheCreatorFactory for [" << name << "]" << std::endl;
        Log::error() << "TransFactories are:" << std::endl;
        for ( j = m->begin(); j != m->end(); ++j )
            Log::error() << "   " << ( *j ).first << std::endl;
        throw eckit::SeriousBug( std::string( "No LegendreCacheCreatorFactory called " ) + name );
    }
    return *j->second;
}

}  // namespace

LegendreCacheCreatorFactory::LegendreCacheCreatorFactory( const std::string& name ) : name_( name ) {
    pthread_once( &once, init );

    eckit::AutoLock<eckit::Mutex> lock( local_mutex );

    ASSERT( m->find( name ) == m->end() );
    ( *m )[name] = this;
}

LegendreCacheCreatorFactory::~LegendreCacheCreatorFactory() {
    eckit::AutoLock<eckit::Mutex> lock( local_mutex );
    m->erase( name_ );
}

bool LegendreCacheCreatorFactory::has( const std::string& name ) {
    pthread_once( &once, init );

    eckit::AutoLock<eckit::Mutex> lock( local_mutex );

    static force_link static_linking;

    return ( m->find( name ) != m->end() );
}

void LegendreCacheCreatorFactory::list( std::ostream& out ) {
    pthread_once( &once, init );

    eckit::AutoLock<eckit::Mutex> lock( local_mutex );

    static force_link static_linking;

    const char* sep = "";
    for ( std::map<std::string, LegendreCacheCreatorFactory*>::const_iterator j = m->begin(); j != m->end(); ++j ) {
        out << sep << ( *j ).first;
        sep = ", ";
    }
}

LegendreCacheCreator::Implementation* LegendreCacheCreatorFactory::build( const Grid& grid, int truncation,
                                                                          const eckit::Configuration& config ) {
    pthread_once( &once, init );

    eckit::AutoLock<eckit::Mutex> lock( local_mutex );

    static force_link static_linking;

    util::Config options = Trans::config();
    options.set( config );

    std::string name = options.getString( "type" );

    Log::debug() << "Looking for LegendreCacheCreatorFactory [" << name << "]" << std::endl;

    return factory( name ).make( grid, truncation, options );
}

LegendreCacheCreator::LegendreCacheCreator() {}

LegendreCacheCreator::LegendreCacheCreator( Implementation* impl ) : impl_( impl ) {}

LegendreCacheCreator::LegendreCacheCreator( const Grid& grid, int truncation, const eckit::Configuration& config ) :
    impl_( LegendreCacheCreatorFactory::build( grid, truncation, config ) ) {}

LegendreCacheCreator::LegendreCacheCreator( const LegendreCacheCreator& creator ) : impl_( creator.impl_ ) {}

bool LegendreCacheCreator::supported() const {
    return impl_->supported();
}

std::string LegendreCacheCreator::uid() const {
    return impl_->uid();
}

void LegendreCacheCreator::create( const std::string& path ) const {
    impl_->create( path );
}

Cache LegendreCacheCreator::create() const {
    return impl_->create();
}

size_t LegendreCacheCreator::estimate() const {
    return impl_->estimate();
}

}  // namespace trans
}  // namespace atlas
