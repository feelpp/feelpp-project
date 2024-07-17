# ##  TEMPLATE.txt.tpl; coding: utf-8 ---

# Author(s): Christophe Prud'homme <christophe.prudhomme@feelpp.org>
# Date: 2024-07-17
#
# Copyright (C) 2024 University of Strasbourg
#
# Distributed under the GPL(GNU Public License):
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#
macro(zip_files src dest)
    # Ensure the destination directory exists
    get_filename_component(dest_dir ${dest} DIRECTORY)
    file(MAKE_DIRECTORY ${dest_dir})

    # Collect all files in the source directory to ensure dependencies are tracked
    file(GLOB_RECURSE src_files ${src}/* ${src}.json)
    get_filename_component(src_dir ${src} NAME)
    message(STATUS "src_dir=${src_dir}")

    add_custom_command(
        OUTPUT ${dest}
        COMMAND ${CMAKE_COMMAND} -E chdir ${src}/.. ${CMAKE_COMMAND} -E tar "cf" ${dest} --format=zip ${src_dir} ${src_dir}.json
        DEPENDS ${src_files}
        COMMENT "Creating zip file ${dest}"
    )
endmacro()

# Usage of the macro to zip the content of the 'fin' folder
set(src_dir ${CMAKE_CURRENT_SOURCE_DIR}/fin)
set(dest_zip ${CMAKE_BINARY_DIR}/datasets/fin.zip)

# List of dataset directories
set(datasets "fin" )

# Loop over the datasets and create a zip for each
foreach(dataset ${datasets})
    set(src_dir ${CMAKE_CURRENT_SOURCE_DIR}/${dataset})
    set(dest_zip ${CMAKE_BINARY_DIR}/datasets/dataset-${dataset}.zip)
    zip_files(${src_dir} ${dest_zip})

    # Add a custom target for each zip file
    add_custom_target(zip_${dataset} ALL DEPENDS ${dest_zip})
endforeach()

