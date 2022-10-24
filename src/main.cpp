/**
 * @file laplacian.cpp
 * @author Christophe Prud'homme <christophe.prudhomme@cemosis.fr>
 * @brief Laplacian
 * @version 0.1
 * @date 2022-10-22
 *
 * @copyright Copyright (c) 2022 Université de Strasbourg
 *
 */
#include <iostream>

#include <feel/feelcore/environment.hpp>
#include <feel/feelcore/json.hpp>
#include <feel/feelcore/ptreetools.hpp>
#include <feel/feelcore/utility.hpp>
#include <feel/feeldiscr/pch.hpp>
#include <feel/feelfilters/exporter.hpp>
#include <feel/feelfilters/loadmesh.hpp>
#include <feel/feelvf/form.hpp>
#include <feel/feelvf/vf.hpp>
#include <feel/feelvf/bdf.hpp>

namespace Feel
{
inline Feel::po::options_description
makeOptions()
{
    Feel::po::options_description options( "rht options" );
    options.add_options()

        // mesh parameters
        ( "specs", Feel::po::value<std::string>(),
          "json spec file for rht" )

        ( "steady", Feel::po::value<bool>()->default_value( 1 ),
          "if 1: steady else unsteady" );

    return options.add( Feel::feel_options() );
}


template <int Dim = FEELPP_DIM, int Order = FEELPP_ORDER>
int runLaplacian( nl::json const& specs )
{
    auto mesh = loadMesh( _mesh = new Mesh<Simplex<Dim>>, _filename = specs["/Meshes/laplacian/Import/filename"_json_pointer].get<std::string>() );
    auto Xh = Pch<Order>( mesh );

    auto u = Xh->element();
    auto v = Xh->element();

    auto a = form2( _test = Xh, _trial = Xh );
    auto at = form2( _test = Xh, _trial = Xh );
    auto l = form1( _test = Xh );
    auto lt = form1( _test = Xh );

    auto M_bdf = bdf( _space = Xh );

    M_bdf->start();

    // from now if the option "steady" is set to True then M_ bdf-setSteady will set time-step=time-final
    if ( boption("steady") )
        M_bdf->setSteady();

    for ( auto [key, material] : specs["/Models/laplacian/materials"_json_pointer].items() )
    {
        LOG( INFO ) << fmt::format( "material {}", material );
        std::string mat = fmt::format( "/Materials/{}/k", material.get<std::string>() );
        auto k = specs[nl::json::json_pointer( mat )].get<std::string>();

        a += integrate( _range = markedelements( mesh, material.get<std::string>() ), 
                _expr = M_bdf->polyDerivCoefficient( 0 ) * expr( k ) * gradt( u ) * trans( grad( v ) ) );
    }

    // BC Neumann
    if ( specs["/BoundaryConditions/laplacian"_json_pointer].contains( "flux" ) )
    {
        for ( auto& [bc, value] : specs["/BoundaryConditions/heat/flux"_json_pointer].items() )
        {
            LOG( INFO ) << fmt::format( "flux {}: {}", bc, value.dump() );
            auto flux = value["expr"].get<std::string>();

            l += integrate( _range = markedfaces( mesh, bc ),
                    _expr = M_bdf->polyDerivCoefficient( 0 ) * expr( flux ) * id( v ) );
        }
    }

    // BC Robin
    if ( specs["/BoundaryConditions/laplacian"_json_pointer].contains( "convective_heat_flux" ) )
    {
        for ( auto& [bc, value] : specs["/BoundaryConditions/heat/convective_heat_flux"_json_pointer].items() )
        {
            LOG( INFO ) << fmt::format( "convective_heat_flux {}: {}", bc, value.dump() );
            auto h = value["h"].get<std::string>();
            auto Text = value["Text"].get<std::string>();

            a += integrate( _range = markedfaces( mesh, bc ),
                    _expr = M_bdf->polyDerivCoefficient( 0 ) * expr( h ) * id( v ) * idt( u ) );
            l += integrate( _range = markedfaces( mesh, bc ),
                    _expr = M_bdf->polyDerivCoefficient( 0 ) * expr( h ) * expr( Text ) * id( v ) );
        }
    }

    M_bdf->initialize( u );

    if ( boption("steady") )
        std::cout << "\n***** Steady state *****" << std::endl;
    else
        std::cout << "The step is  " << M_bdf->timeStep() << "\n"
                  << "The initial time is " << M_bdf->timeInitial() << "\n"
                  << "The final time is " << M_bdf->timeFinal() << "\n"
                  << "BDF order :  " << M_bdf->timeOrder() << "\n" << std::endl;

    // Solve
    for ( M_bdf->start(); M_bdf->isFinished()==false; M_bdf->next(u) )
    {
        at = a;
        lt = l;

        for ( auto [key, material] : specs["/Models/laplacian/materials"_json_pointer].items() )
        {
            std::string matRho = fmt::format( "/Materials/{}/rho", material.get<std::string>() );
            std::string matCp = fmt::format( "/Materials/{}/Cp", material.get<std::string>() );
            auto Rho = specs[nl::json::json_pointer( matRho )].get<std::string>();
            auto Cp = specs[nl::json::json_pointer( matCp )].get<std::string>();

            lt += integrate( _range = markedelements( mesh, material.get<std::string>() ), 
                    _expr = expr( Rho ) * expr( Cp ) * idv( M_bdf->polyDeriv() ) * id( v ) );
        }

        at.solve( _rhs = lt, _solution = u );
    }

    // compute outputs
    auto m = mean( _range = elements( mesh ), _expr = idv( u ) );
    if ( Environment::isMasterRank() )
    {
        std::cout << fmt::format( "- mean value: {}", m ) << std::endl;
        std::cout << fmt::format( "-  min value: {}", u.min() ) << std::endl;
        std::cout << fmt::format( "-  max value: {}", u.max() ) << std::endl;
        std::cout << fmt::format( "-  max deviation: {}", u.max() - u.min() ) << std::endl;
    }
    // Export
    auto e = exporter( _mesh = mesh );
    e->addRegions();
    e->add( "T", v );
    e->save();

    return 0;
}
} // namespace Feel

int main( int argc, char** argv )
{
    using namespace Feel;
    try
    {
        Environment env( _argc = argc, _argv = argv,
                         _desc = makeOptions(),
                         _about = about( _name = fmt::format( "laplacian-{}dp{}", FEELPP_DIM, FEELPP_ORDER ),
                                         _author = "Feel++ Consortium",
                                         _email = "feelpp@cemosis.fr" ) );
        auto jsonfile = removeComments( readFromFile( Environment::expand( soption( "specs" ) ) ) );
        std::istringstream istr( jsonfile );
        json specs = json::parse( istr );
        runLaplacian<FEELPP_DIM, FEELPP_ORDER>( specs );
    }
    catch ( ... )
    {
        handleExceptions();
    }
    return 1;
}