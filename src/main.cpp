/* -*- mode: c++; coding: utf-8; tab-width: 4; indent-tabs-mode: nil;
 c-basic-offset: 4; show-trailing-whitespace: t  -*-

 This file is part of the Feel library

 Author(s): Christophe Prud'homme <christophe.prudhomme@feelpp.org>
 Date: 2014-06-03

 Copyright (C) 2014-2019 Cemosis - University of Strasbourg

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <feel/feelcore/environment.hpp>

int main( int argc, char **argv )
{
  //# marker1 #
  using namespace Feel;
  po::options_description laplacianoptions( "Laplacian options" );
  laplacianoptions.add_options()( "polymap.study", po::value<std::vector<std::string>>(), "study name" )
      ("polymap.hbfz.solution", po::value<std::string>(), "solution filename" )
      ("polymap.polygon", po::value<std::string>()->default_value("${holo3_srcdir}/src/feelpp/surface_reconstruction/surface_rec/poly.txt" ),   "polygon filename" )
      ( "polymap.pixelsize", po::value<double>()->default_value( 4.5e-3 ), "pixel size (in mm)" )
      ( "polymap.withCoord", po::value<bool>()->default_value( false ),  "use hbf coordinates files" )
      ( "polymap.extraction", po::value<bool>()->default_value( false ), "extraction mode" )
      ( "polymap.pmean", po::value<bool>()->default_value( true ), "remove mean of slopes" )
      ( "polymap.export.paraview", po::value<bool>()->default_value( false ), "export reconstruction in paraview format" )
      ( "polymap.export.hbf", po::value<bool>()->default_value( true ), "export reconstruction in hbf format" )
      ( "polymap.levelset", po::value<bool>()->default_value( false ), "levelset mode" );

  Environment env( _argc = argc, _argv = argv, _desc = laplacianoptions,
                   _about = about( _name = "surface_rec",
                                   _author = "Feel++ Consortium",
                                   _email = "feelpp-devel@feelpp.org" ) );

}