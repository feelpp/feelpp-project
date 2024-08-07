//! -*- mode: c++; coding: utf-8; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; show-trailing-whitespace: t  -*- vim:fenc=utf-8:ft=cpp:et:sw=4:ts=4:sts=4
//!
//! This file is part of the Feel++ library
//!
//! This library is free software; you can redistribute it and/or
//! modify it under the terms of the GNU Lesser General Public
//! License as published by the Free Software Foundation; either
//! version 2.1 of the License, or (at your option) any later version.
//!
//! This library is distributed in the hope that it will be useful,
//! but WITHOUT ANY WARRANTY; without even the implied warranty of
//! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//! Lesser General Public License for more details.
//!
//! You should have received a copy of the GNU Lesser General Public
//! License along with this library; if not, write to the Free Software
//! Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//!
//! @file
//! @author Christophe Prud'homme <christophe.prudhomme@cemosis.fr>
//! @date 2023-10-31
//! @copyright 2023 Feel++ Consortium
//! @copyright 2023 Université de Strasbourg
//!
#pragma once
#include <iostream>

#include <feel/feelalg/matrixpetsc.hpp>
#include <feel/feelalg/topetsc.hpp>
#include <feel/feelalg/vectorpetsc.hpp>
#include <feel/feelcore/environment.hpp>
#include <feel/feelcore/json.hpp>
#include <feel/feelcore/ptreetools.hpp>
#include <feel/feelcore/utility.hpp>
#include <feel/feeldiscr/minmax.hpp>
#include <feel/feeldiscr/pch.hpp>
#include <feel/feelfilters/exporter.hpp>
#include <feel/feelfilters/loadmesh.hpp>
#include <feel/feelts/bdf.hpp>
#include <feel/feelvf/form.hpp>
#include <feel/feelvf/measure.hpp>
#include <feel/feelvf/vf.hpp>
#include <fmt/core.h>
#include <fmt/ostream.h>

namespace Feel
{
inline const int FEELPP_DIM=2;
inline const int FEELPP_ORDER=1;

static inline const bool do_print = true;
static inline const bool dont_print = false;

template<int M, int N=M>
inline
Expr<vf::detail::Ones<M,N> >
constant( Eigen::Matrix<double,M,N> const& value )
{
    return Expr<vf::detail::Ones<M,N> >( vf::detail::Ones<M, N>( value ));
}

/**
 * @brief compute the summary of a container
 *
 * @tparam Container type of the container
 * @param c container
 * @param print boolean, true print the summary, false otherwise
 * @return nl::json json object containing the summary
 */
template<typename Container>
nl::json summary( Container const& c, bool print = do_print )
{
    using namespace Feel;
    using namespace Feel::vf;
    nl::json j;
    j["size"] = c.size();
    auto r = minmaxelt(_range = elements(support(c.functionSpace())), _element = c);
    j["min"] = r[0];
    j["max"] = r[1];
    j["mean"] = mean( _range = elements( c.mesh() ), _expr = idv( c ) );

    if (print)
    {
        if (Environment::isMasterRank())
            std::cout << j.dump(2) << std::endl;
    }
    return j;
}
inline Feel::po::options_description
makeOptions()
{
    Feel::po::options_description options( "laplacian options" );
    options.add_options()

        // mesh parameters
        ( "specs", Feel::po::value<std::string>(),
          "json spec file for rht" )

        ( "steady", Feel::po::value<bool>()->default_value( 1 ),
          "if 1: steady else unsteady" );

    return options.add( Feel::feel_options() );
}

template<typename T>
T get_value(const nl::json& specs, const std::string& path, const T& default_value)
{
    auto json_pointer = nl::json::json_pointer(path);
    return specs.contains(json_pointer) ? specs[json_pointer].get<T>() : default_value;
}

template <int Dim, int Order>
class Laplacian
{
public:
    using mesh_t = Mesh<Simplex<Dim>>;
    using space_t = Pch_type<mesh_t, Order>;
    using space_ptr_t = Pch_ptrtype<mesh_t, Order>; // Define the type for Pch_ptrtype
    using element_t = typename space_t::element_type;
    using form2_type = form2_t<space_t,space_t>; // Define the type for form2
    using form1_type = form1_t<space_t>; // Define the type for form1
    using bdf_ptrtype = std::shared_ptr<Bdf<space_t>>;
    using exporter_ptrtype = std::shared_ptr<Exporter<mesh_t>>; // Define the type for exporter_ptrtype
    using matrix_ptr_t = std::shared_ptr<MatrixPetsc<double>>;
    using vector_ptr_t = std::shared_ptr<VectorPetsc<double>>;

    Laplacian() = default;
    Laplacian( Laplacian const& l );
    Laplacian( Laplacian && l ) noexcept;
    Laplacian(nl::json const& specs);

    Laplacian& operator=( Laplacian const& l );

    // Accessors
    nl::json const& specs() const { return specs_; }
    std::shared_ptr<mesh_t> const& mesh() const { return mesh_; }
    space_ptr_t const& Xh() const { return Xh_; }
    element_t& u() { return u_; }
    element_t const& u() const { return u_; }
    element_t const& v() const { return v_; }
    form2_type const& a() const { return a_; }
    form2_type const& at() const { return at_; }
    form1_type const& l() const { return l_; }
    form1_type const& lt() const { return lt_; }
    bdf_ptrtype const& bdf() const { return bdf_; }
    exporter_ptrtype const& exporter() const { return e_; }
    nl::json measures() const { return meas_; }

    // Mutators
    void setSpecs(nl::json const& specs) { specs_ = specs; }
    void setMesh(std::shared_ptr<mesh_t> const& mesh) { mesh_ = mesh; }
    void setU(element_t const& u) { u_ = u; }

    void initialize();
    void processMaterials();
    void processBoundaryConditions();
    void run();
    void timeLoop();
    nl::json exportResults() const { return exportResults( bdf_->time(), u_ ); }
    nl::json exportResults( double t, element_t const& u ) const;
    void summary(/*arguments*/);
    void writeResultsToFile(const std::string& filename) const;
    form2_type assembleGradGrad( std::vector<std::string> const& markers, Eigen::MatrixXd const& coeffs );
    form2_type assembleMass( std::vector<std::string> const& markers, double coeff );
    form1_type assembleFlux( std::vector<std::string> const& markers, double coeff );

    // Accessors and mutators for members
    /* ... */

private:
    nl::json specs_;
    std::shared_ptr<mesh_t> mesh_;
    space_ptr_t Xh_;
    element_t u_, v_;
    form2_type a_, at_;
    form1_type l_, lt_;
    bdf_ptrtype bdf_;
    mutable exporter_ptrtype e_;
    mutable nl::json meas_;
};

// Constructor
template <int Dim, int Order>
Laplacian<Dim, Order>::Laplacian(nl::json const& specs) : specs_(specs)
{
    initialize();
}
template <int Dim, int Order>
Laplacian<Dim, Order>::Laplacian( Laplacian const& l )
    : specs_( l.specs ),
      mesh_( l.mesh_ ),
      Xh_( l.Xh_ ),
      u_( l.u_ ),
      v_( l.v_ ),
      a_( form2( _test = Xh_, _trial = Xh_ ) ),
      at_( form2( _test = Xh_, _trial = Xh_ ) ),
      l_( form1( _test = Xh_ ) ),
      lt_( form1( _test = Xh_ ) ),
      bdf_( l.bdf_ ),
      e_( Feel::exporter( _mesh = mesh_ ) ),
      meas_( l.meas_ )
{
    a_ = l.a_;
    at_ = l.at_;
    l_ = l.l_;
    lt_ = l.lt_;
}

template <int Dim, int Order>
Laplacian<Dim, Order>::Laplacian( Laplacian&& l ) noexcept
    : specs_( std::move( l.specs_ ) ),
      mesh_( std::move( l.mesh_ ) ),
      Xh_( std::move( l.Xh_ ) ),
      u_( std::move( l.u_ ) ),
      v_( std::move( l.v_ ) ),
      a_( std::move( l.a_ ) ),
      at_( std::move( l.at_ ) ),
      l_( std::move( l.l_ ) ),
      lt_( std::move( l.lt_ ) ),
      bdf_( std::move( l.bdf_ ) ),
      e_( std::move( l.e_ ) ),
      meas_( std::move( l.meas_ ) )
{
    // Optionally, handle the moved-from state if necessary
}
template <int Dim, int Order>
Laplacian<Dim, Order>&
Laplacian<Dim, Order>::operator=( Laplacian const& l )
{
    if ( this != &l )
    {
        specs_ = l.specs_;
        mesh_ = l.mesh_;
        Xh_ = l.Xh_;
        u_ = l.u_;
        v_ = l.v_;
        a_ = l.a_;
        at_ = l.at_;
        l_ = l.l_;
        lt_ = l.lt_;
        bdf_ = l.bdf_;
        e_ = exporter( _mesh = mesh_ );
        meas_ = l.meas_;
    }
    return *this;
}


// Initialization
template <int Dim, int Order>
void Laplacian<Dim, Order>::initialize()
{
    // Load mesh and initialize Xh, a, l, etc.
    mesh_ = loadMesh( _mesh = new mesh_t, _filename = specs_["/Meshes/laplacian/Import/filename"_json_pointer].get<std::string>() );
    // define Xh on a marked region
    if ( specs_["/Spaces/laplacian/Domain"_json_pointer].contains("marker") )
        Xh_ = Pch<Order>(mesh_, markedelements(mesh_, specs_["/Spaces/laplacian/Domain/marker"_json_pointer].get<std::vector<std::string>>()));
    // define Xh via a levelset phi where phi < 0 defines the Domain and phi = 0 the boundary
    else if (specs_["/Spaces/laplacian/Domain"_json_pointer].contains("levelset"))
        Xh_ = Pch<Order>(mesh_, elements(mesh_, expr(specs_["/Spaces/laplacian/Domain/levelset"_json_pointer].get<std::string>())));
    // define Xh on the whole mesh
    else
        Xh_ = Pch<Order>(mesh_);

    u_ = Xh_->element();
    v_ = Xh_->element();

    a_ = form2( _test = Xh_, _trial = Xh_ );
    at_ = form2( _test = Xh_, _trial = Xh_ );
    l_ = form1( _test = Xh_ );
    lt_ = form1( _test = Xh_ );

    bool steady = get_value(specs_, "/TimeStepping/laplacian/steady", true);
    int time_order = get_value(specs_, "/TimeStepping/laplacian/order", 2);
    double initial_time = get_value(specs_, "/TimeStepping/laplacian/start", 0.0);
    double final_time = get_value(specs_, "/TimeStepping/laplacian/end", 1.0);
    double time_step = get_value(specs_, "/TimeStepping/laplacian/step", 0.1);
    bdf_ = Feel::bdf( _space = Xh_, _steady=steady, _initial_time=initial_time, _final_time=final_time, _time_step=time_step, _order=time_order );

    bdf_->start();
    if ( steady )
        bdf_->setSteady();

    bdf_->initialize( u_ );

    if ( steady )
        std::cout << "\n***** Compute Steady state *****" << std::endl;
    else
    {
        std::cout << "\n***** Compute Transient state *****" << std::endl;
        std::cout << "The step is  " << bdf_->timeStep() << "\n"
                  << "The initial time is " << bdf_->timeInitial() << "\n"
                  << "The final time is " << bdf_->timeFinal() << "\n"
                  << "BDF order :  " << bdf_->timeOrder() << "\n" << std::endl
                  << "BDF coeff :  " << bdf_->polyDerivCoefficient( 0 ) << "\n" << std::endl;
    }

    a_.zero();
    at_.zero();
    l_.zero();
    lt_.zero();

    e_ = Feel::exporter(_mesh = mesh_);
}

// Process materials
template <int Dim, int Order>
void Laplacian<Dim, Order>::processMaterials()
{
    for ( auto [key, material] : specs_["/Models/laplacian/Materials"_json_pointer].items() )
    {
        LOG( INFO ) << fmt::format( "Material {} found", material.dump() );
        std::string mat = fmt::format( "/Materials/{}/k", material.get<std::string>() );
        auto k = specs_[nl::json::json_pointer( mat )].get<std::string>();
        std::string matRho = fmt::format( "/Materials/{}/rho", material.get<std::string>() );
        auto Rho = specs_[nl::json::json_pointer( matRho )].get<std::string>();
        std::string matCp = fmt::format( "/Materials/{}/Cp", material.get<std::string>() );
        auto Cp = specs_[nl::json::json_pointer( matCp )].get<std::string>();

        a_ += integrate( _range = markedelements( support( Xh_ ), material.get<std::string>() ),
                _expr = bdf_->polyDerivCoefficient( 0 ) * expr( Rho ) * expr( Cp ) * idt( u_ ) * id( v_ ) + expr( k ) * gradt( u_ ) * trans( grad( v_ ) ) );
    }
}

// Process boundary conditions
template <int Dim, int Order>
void Laplacian<Dim, Order>::processBoundaryConditions()
{
    // BC Neumann
    if ( specs_["/BoundaryConditions/laplacian"_json_pointer].contains( "flux" ) )
    {
        for ( auto& [bc, value] : specs_["/BoundaryConditions/laplacian/flux"_json_pointer].items() )
        {
            LOG( INFO ) << fmt::format( "flux {}: {}", bc, value.dump() );
            auto flux = value["expr"].get<std::string>();

            l_ += integrate( _range = markedfaces( support( Xh_ ), bc ),
                    _expr = expr( flux ) * id( v_ ) );
        }
    }

    // BC Robin
    if ( specs_["/BoundaryConditions/laplacian"_json_pointer].contains( "convective_laplacian_flux" ) )
    {
        for ( auto& [bc, value] : specs_["/BoundaryConditions/laplacian/convective_laplacian_flux"_json_pointer].items() )
        {
            LOG( INFO ) << fmt::format( "convective_laplacian_flux {}: {}", bc, value.dump() );
            auto h = value["h"].get<std::string>();
            auto Text = value["Text"].get<std::string>();

            a_ += integrate( _range = markedfaces( support( Xh_ ), bc ),
                    _expr = expr( h ) * id( v_ ) * idt( u_ ) );
            l_ += integrate( _range = markedfaces( support( Xh_ ), bc ),
                    _expr = expr( h ) * expr( Text ) * id( v_ ) );
        }
    }
}

// Run method (main method to run Laplacian process)
template <int Dim, int Order>
void Laplacian<Dim, Order>::run()
{
    initialize();
    processMaterials();
    processBoundaryConditions();
    timeLoop();
    exportResults();
}

// Time loop
template <int Dim, int Order>
void Laplacian<Dim, Order>::timeLoop()
{
    // time loop
    for ( bdf_->start(); bdf_->isFinished()==false; bdf_->next(u_) )
    {
        at_ = a_;
        lt_ = l_;

        for ( auto [key, material] : specs_["/Models/laplacian/Materials"_json_pointer].items() )
        {
            std::string matRho = fmt::format( "/Materials/{}/rho", material.get<std::string>() );
            std::string matCp = fmt::format( "/Materials/{}/Cp", material.get<std::string>() );
            auto Rho = specs_[nl::json::json_pointer( matRho )].get<std::string>();
            auto Cp = specs_[nl::json::json_pointer( matCp )].get<std::string>();

            lt_ += integrate( _range = markedelements( support( Xh_ ), material.get<std::string>() ),
                    _expr = expr( Rho ) * expr( Cp ) * idv( bdf_->polyDeriv() ) * id( v_ ) );
        }

        at_.solve( _rhs = lt_, _solution = u_ );

        this->exportResults();
    }
}

// Export results
template <int Dim, int Order>
nl::json Laplacian<Dim, Order>::exportResults( double t, element_t const& u ) const
{
    e_->step(t)->addRegions();
    e_->step(t)->add("u", u);
    e_->save();

    auto totalQuantity = integrate(_range=elements(mesh_), _expr=idv(u)).evaluate()(0,0);
    auto totalFlux = integrate(_range=boundaryfaces(mesh_), _expr=gradv(u)*N()).evaluate()(0,0);
    double meas=measure(_range=elements(mesh_), _expr=cst(1.0));
    meas_["time"].push_back(t);
    meas_["totalQuantity"].push_back(totalQuantity);
    meas_["totalFlux"].push_back(totalFlux);
    meas_["mean"].push_back(totalQuantity/meas);
    meas_["min"].push_back(u.min());
    meas_["max"].push_back(u.max());
    for( auto [key,values] : mesh_->markerNames())
    {
        if ( values[1] == Dim )
        {
            double meas=measure(_range=markedelements(mesh_,key), _expr=cst(1.0));
            auto quantity = integrate(_range=markedelements(mesh_,key), _expr=idv(u)).evaluate()(0,0);
            meas_[fmt::format("quantity_{}",key)].push_back(quantity);
            meas_[fmt::format("mean_{}",key)].push_back(quantity/meas);
        }
        else if ( values[1] == Dim-1 )
        {
            double meas=measure(_range=markedfaces(mesh_,key), _expr=cst(1.0));
            auto quantity = integrate(_range=markedfaces(mesh_,key), _expr=idv(u)).evaluate()(0,0);
            meas_[fmt::format("quantity_{}",key)].push_back(quantity);
            meas_[fmt::format("mean_{}",key)].push_back(quantity/meas);
            auto flux = integrate(_range=markedfaces(mesh_,key), _expr=gradv(u)*N()).evaluate()(0,0);
            meas_[fmt::format("flux_{}",key)].push_back(flux);
        }

    }
    return meas_;
}
template <int Dim, int Order>
void Laplacian<Dim, Order>::writeResultsToFile(const std::string& filename) const
{
    std::ofstream file(filename);
    if (file.is_open()) {
        file << meas_.dump(4);  // Indent of 4 spaces for readability
        file.close();
    } else {
        std::cerr << "Unable to open file: " << filename << std::endl;
    }
}

// Summary method
template <int Dim, int Order>
void Laplacian<Dim, Order>::summary(/*arguments*/) {
    /* ... summary code ... */
}

template <int Dim, int Order>
typename Laplacian<Dim, Order>::form2_type
Laplacian<Dim, Order>::assembleGradGrad( std::vector<std::string> const& markers, Eigen::MatrixXd const& coeffs )
{
    auto a = form2( _test = Xh_, _trial = Xh_ );
    for( auto marker : markers )
    {
        LOG( INFO ) << fmt::format( "assemble grad.grad on marker {} with coeffs: {}", marker, coeffs );
        a += integrate( _range = markedelements( support( Xh_ ), marker ),
                        _expr = trans(constant<Dim,Dim>(coeffs) * trans(gradt( u_ ))) * trans(grad( v_ )) );
    }
    a.close();
    return a;
}
template <int Dim, int Order>
typename Laplacian<Dim, Order>::form2_type
Laplacian<Dim, Order>::assembleMass( std::vector<std::string> const& markers, double coeff )
{
    auto a = form2( _test = Xh_, _trial = Xh_ );
    for( auto marker : markers )
    {
        if ( mesh_->markerNames().at(marker)[1] == Dim )
        {
            LOG( INFO ) << fmt::format( "assemble mass on volume marker {} with coeff: {}", marker, coeff );
            a += integrate( _range = markedelements( support( Xh_ ), marker ),
                        _expr = coeff * idt( u_ ) * id( v_ ) );
        }
        else if ( mesh_->markerNames().at(marker)[1] == Dim-1 )
        {
            LOG( INFO ) << fmt::format( "assemble mass on face marker {} with coeff: {}", marker, coeff );
            a += integrate( _range = markedfaces( support( Xh_ ), marker ),
                        _expr = coeff * idt( u_ ) * id( v_ ) );
        }
    }
    a.close();
    return a;
}
template <int Dim, int Order>
typename Laplacian<Dim, Order>::form1_type
Laplacian<Dim, Order>::assembleFlux( std::vector<std::string> const& markers, double coeff )
{
    auto l = form1( _test = Xh_ );
    for( auto marker : markers )
    {
        if ( mesh_->markerNames().at(marker)[1] == Dim )
        {
            LOG( INFO ) << fmt::format( "assemble flux on volume marker {} with coeff: {}", marker, coeff );
            l += integrate( _range = markedelements( support( Xh_ ), marker ),
                            _expr = coeff * id( v_ ) );
        }
        else
        {
            LOG(INFO) << fmt::format("assemble flux on marker {} is a face marker with coeff: {}", marker, coeff);
            l += integrate( _range = markedfaces( support( Xh_ ), marker ),
                            _expr = coeff * id( v_ ) );
        }
    }
    l.close();
    v_.setConstant(1);
    LOG(INFO) << fmt::format("flux l(1)={}",l(v_)) << std::endl;
    return l;
}

} // namespace Feel
