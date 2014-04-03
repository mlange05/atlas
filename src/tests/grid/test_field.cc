/*
 * (C) Copyright 1996-2012 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/log/Log.h"
#include "eckit/runtime/Tool.h"

#include "atlas/FunctionSpace.hpp"
#include "atlas/grid/Grid.h"
#include "atlas/grid/LatLon.h"
#include "atlas/grid/Field.h"
#include "atlas/grid/Tesselation.h"

using namespace std;
using namespace eckit;
using namespace atlas;

//-----------------------------------------------------------------------------

namespace eckit_test {

//-----------------------------------------------------------------------------

class TestField : public Tool {
public:

    TestField(int argc,char **argv): Tool(argc,argv) {}

    ~TestField() {}

    virtual void run();

    void test_constructor();
};

//-----------------------------------------------------------------------------

void TestField::test_constructor()
{
    using namespace atlas::grid;

    // create a grid

    Grid::BoundBox earth ( Grid::Point(-90.,0.), Grid::Point(90.,360.) );

    Grid::Ptr g (new LatLon( 4, 4, earth ) );

    // create some reference data for testing

    std::vector<double> ref_data;
    ref_data.resize( g->nPoints() );
    for(size_t i = 0; i < ref_data.size(); ++i)
        ref_data.push_back((double)i);

    // now build a test field handle

    std::string sname("field_name");
    ASSERT( g );

    Tesselation::create_mesh_structure( g->mesh(), g->nPoints() );

    Mesh& mesh = g->mesh();

    ASSERT( mesh.has_function_space("nodes") );

    MetaData::Ptr md( new MetaData() );

    ASSERT( md );

    atlas::FunctionSpace& nodes  = mesh.function_space( "nodes" );

    FieldT<double>& data = nodes.create_field<double>( sname,1);

    for(size_t i = 0; i < ref_data.size(); i++)
        data[i] = ref_data[i];

    // create field handle

    FieldH::Ptr f( new FieldH( g, std::move(md), nodes.field<double>( sname ) ) );

    ASSERT( f );

    FieldH::Vector fields;
    fields.push_back(f);

    FieldSet fs(fields);
    
    // iterate over the fields
    for (FieldH::Vector::iterator it = fs.fields().begin(); it != fs.fields().end(); ++it)
    {
        // extract and test the data
        FieldH::Data& d = (*it)->data();
        for (size_t i = 0; i < ref_data.size(); i++)
        {
            ASSERT(ref_data[i] == d[i]);
        }   
    }
}

//-----------------------------------------------------------------------------

void TestField::run()
{
    test_constructor();
}

//-----------------------------------------------------------------------------

} // namespace eckit_test

//-----------------------------------------------------------------------------

int main(int argc,char **argv)
{
    eckit_test::TestField mytest(argc,argv);
    mytest.start();
    return 0;
}

