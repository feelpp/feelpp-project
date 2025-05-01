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
//! @brief Python bindings using pybind11 for the Laplacian example
//! @author Christophe Prud'homme <christophe.prudhomme@cemosis.fr>
//! @date 2023-10-31
//! @copyright 2023-2024 Feel++ Consortium
//! @copyright 2023 Université de Strasbourg
//!

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
//! @author Christophe Prud'homme <christophe.prudhomme@feelpp.org>
//! @date 15 Jun 2017
//! @copyright 2017 Feel++ Consortium
//!



#include "laplacian.hpp"
#if defined( FEELPP_HAS_PETSC4PY )
#include <petsc4py/petsc4py.h>
#endif
#include <feel/feelalg/backend.hpp>
#include <feel/feelalg/backendpetsc.hpp>
#include <feel/feelalg/matrixpetsc.hpp>
#include <feel/feelalg/topetsc.hpp>
#include <feel/feelalg/vectorpetsc.hpp>
#include <feel/feelalg/vectorublas.hpp>
#include <feel/feelcore/environment.hpp>
#include <feel/feelcore/json.hpp>
#include <feel/feelpython/pybind11/eigen.h>
#include <feel/feelpython/pybind11/json.h>
#include <feel/feelpython/pybind11/pybind11.h>
#include <feel/feelpython/pybind11/stl.h>
#include <feel/feelpython/pybind11/stl_bind.h>
#include <mpi4py/mpi4py.h>


namespace py = pybind11;

template<int Dim,int Order>
void
laplacian_inst( py::module &m )
{
    using namespace Feel;

    py::class_<Laplacian<Dim, Order>>( m, fmt::format( "Laplacian{}DP{}", Dim, Order ).c_str() )
        .def( py::init<>() )
        .def( py::init<const nl::json&>() )
        .def( "initialize", &Laplacian<Dim, Order>::initialize, "Initialize the Laplacian instance" )
        .def( "processMaterials", &Laplacian<Dim, Order>::processMaterials, "Process materials from the json data" )
        .def( "processBoundaryConditions", &Laplacian<Dim, Order>::processBoundaryConditions, "Process boundary conditions from the json data" )
        .def( "run", &Laplacian<Dim, Order>::run, "Run the Laplacian instance" )
        .def( "timeLoop", &Laplacian<Dim, Order>::timeLoop, "Execute the time loop" )
        .def(
            "exportResults", []( Laplacian<Dim, Order> const& l )
            { l.exportResults(); },
            "Postprocess and export the results" )
        .def(
            "exportResults", []( Laplacian<Dim, Order> const& l, typename Laplacian<Dim, Order>::element_t const& u )
            { l.exportResults( 0, u ); },
            "Postprocess and export the results for steady case" )
        .def(
            "exportResults", []( Laplacian<Dim, Order> const& l, double t, typename Laplacian<Dim, Order>::element_t const& u )
            { l.exportResults( t, u ); },
            "Postprocess and export the results at time t" )
        .def( "summary", &Laplacian<Dim, Order>::summary )
        .def( "specs", &Laplacian<Dim, Order>::specs, "Return the json specification of the Laplacian instance" )
        .def( "setSpecs", &Laplacian<Dim, Order>::setSpecs, "Set the json specification of the Laplacian instance" )
        .def( "mesh", &Laplacian<Dim, Order>::mesh, "Return the mesh" )
        .def( "setMesh", &Laplacian<Dim, Order>::setMesh, "Set the mesh" )
        .def( "Xh", &Laplacian<Dim, Order>::Xh, "Return the function space" )
        .def(
            "u", []( Laplacian<Dim, Order>& l )
            { return l.u(); },
            "Return the element u" )
        .def( "setU", &Laplacian<Dim, Order>::setU, "Set the element u" )
        .def( "measures", &Laplacian<Dim, Order>::measures, "Return the measures" )
        .def( "writeResultsToFile", &Laplacian<Dim, Order>::writeResultsToFile, "Write the results to file" )
        .def( "assembleGradGrad", &Laplacian<Dim, Order>::assembleGradGrad, "assemble grad.grad terms", py::arg( "markers" ), py::arg( "coeffs" ) = Eigen::MatrixXd::Ones( Dim, Dim ) )
        .def( "assembleMass", &Laplacian<Dim, Order>::assembleMass, "assemble mass terms", py::arg( "markers" ), py::arg( "coeffs" ) = 1 )
        .def( "assembleFlux", &Laplacian<Dim, Order>::assembleFlux, "assemble flux terms", py::arg( "markers" ), py::arg( "coeffs" ) = 1 );
}
PYBIND11_MODULE(_laplacian, m )
{
    if (import_mpi4py()<0) return ;
    m.doc() = fmt::format("Python bindings for Laplacian class" );  // Optional module docstring
    laplacian_inst<2,1>(m);
    laplacian_inst<2,2>(m);
//    laplacian_inst<2,3>(m);
//    laplacian_inst<3,1>(m);
//    laplacian_inst<3,2>(m);
}
