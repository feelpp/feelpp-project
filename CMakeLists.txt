###  TEMPLATE.txt.tpl; coding: utf-8 ---

#  Author(s): Christophe Prud'homme <christophe.prudhomme@feelpp.org>
#       Date: 2020-06-03
#
#  Copyright (C) 2020-present Cemosis
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
cmake_minimum_required(VERSION 3.21.0)

project(
  feelpp-project
  VERSION 4.0.0 )

set(EXTRA_VERSION "-preview.1")
set(PROJECT_SHORTNAME "fp")

# Package versioning (separate from software version)
# This allows for package updates without changing software version

# Allow override from environment or command line
if(DEFINED ENV{PACKAGE_REVISION})
  set(PACKAGE_REVISION "$ENV{PACKAGE_REVISION}")
elseif(DEFINED ENV{GITHUB_RUN_NUMBER})
  # Better versioning schemes based on context
  if(DEFINED ENV{GITHUB_REF_TYPE} AND "$ENV{GITHUB_REF_TYPE}" STREQUAL "tag")
    # For tagged releases: use build number (e.g., v4.0.0 -> 4.0.0-preview.1-1)
    set(PACKAGE_REVISION "1")
  elseif(DEFINED ENV{GITHUB_REF_NAME} AND "$ENV{GITHUB_REF_NAME}" STREQUAL "master")
    # For master branch: use date-based revision (e.g., 4.0.0-preview.1-20250928.1)
    string(TIMESTAMP DATE_REVISION "%Y%m%d")
    math(EXPR DAILY_BUILD "$ENV{GITHUB_RUN_NUMBER} % 100")  # Daily build number (0-99)
    set(PACKAGE_REVISION "${DATE_REVISION}.${DAILY_BUILD}")
  else()
    # For PR/other branches: use short SHA + run number mod 1000
    if(DEFINED ENV{GITHUB_SHA})
      string(SUBSTRING "$ENV{GITHUB_SHA}" 0 7 SHORT_SHA)
      math(EXPR SHORT_RUN "$ENV{GITHUB_RUN_NUMBER} % 1000")
      set(PACKAGE_REVISION "git${SHORT_SHA}.${SHORT_RUN}")
    else()
      math(EXPR SHORT_RUN "$ENV{GITHUB_RUN_NUMBER} % 1000")
      set(PACKAGE_REVISION "${SHORT_RUN}")
    endif()
  endif()
else()
  set(PACKAGE_REVISION "1")
endif()

# Package epoch (rarely used, for version downgrades)
if(DEFINED ENV{PACKAGE_EPOCH})
  set(PACKAGE_EPOCH "$ENV{PACKAGE_EPOCH}")
else()
  set(PACKAGE_EPOCH "")
endif()

# Construct full package version following Debian policy
if(PACKAGE_EPOCH AND NOT PACKAGE_EPOCH STREQUAL "")
  set(FULL_PACKAGE_VERSION "${PACKAGE_EPOCH}:${PROJECT_VERSION}${EXTRA_VERSION}-${PACKAGE_REVISION}")
else()
  set(FULL_PACKAGE_VERSION "${PROJECT_VERSION}${EXTRA_VERSION}-${PACKAGE_REVISION}")
endif()

# Versioning information
message(STATUS "[feelpp-project] Software Version: ${PROJECT_VERSION}${EXTRA_VERSION}")
message(STATUS "[feelpp-project] Package Version: ${FULL_PACKAGE_VERSION}")
message(STATUS "[feelpp-project] Package Revision: ${PACKAGE_REVISION}")
if(DEFINED ENV{GITHUB_REF_TYPE})
  message(STATUS "[feelpp-project] Build Context: $ENV{GITHUB_REF_TYPE} ($ENV{GITHUB_REF_NAME})")
endif()

include(GNUInstallDirs)

if (POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
  set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
endif()

find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
find_package(pybind11 CONFIG REQUIRED)

find_package(Feel++  REQUIRED COMPONENTS Toolboxes PATHS ${FEELPP_DIR}/share/feelpp/feel/cmake/modules $ENV{FEELPP_DIR}/share/feelpp/feel/cmake/modules /usr/share/feelpp/feel/cmake/modules /usr/local/share/feelpp/feel/cmake/modules )
if(NOT FEELPP_FOUND)
  message(FATAL_ERROR "Feel++ was not found on your system. Make sure to install it and specify the FEELPP_DIR to reference the installation directory.")
endif()
if ( NOT FEELPP_TOOLBOXES_FOUND )
  message(WARNING "Feel++ Toolboxes SDK is not found on your system. The toolbox application is now disabled.")
endif()

enable_testing()

if ( NOT TARGET check )
  add_custom_target(check)
endif()

option( FEELPP_PROJECT_ENABLE_CPP "Enable C++ application" ON )
option( FEELPP_PROJECT_ENABLE_PYTHON "Enable Python application" ON )
option( FEELPP_PROJECT_ENABLE_TESTS "Enable tests" ON )
option( FEELPP_PROJECT_ENABLE_DOCS "Enable documentation" OFF )
add_subdirectory( src )
#add_subdirectory( docs )

# extra ignored files
list(APPEND CPACK_SOURCE_IGNORE_FILES
  .git
  .gitignore
  .travis.yml
  .appveyor.yml
  )

# Map CMAKE_SYSTEM_PROCESSOR to Debian architecture names early
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64")
  set(DEBIAN_ARCH "amd64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
  set(DEBIAN_ARCH "arm64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7|armhf")
  set(DEBIAN_ARCH "armhf")
else()
  set(DEBIAN_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
endif()

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}${EXTRA_VERSION}")

# Use Debian-style package naming with package revision
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}_${FULL_PACKAGE_VERSION}_${DEBIAN_ARCH}")
set(CPACK_PACKAGE_DIRECTORY "${PROJECT_BINARY_DIR}/assets")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Cemosis feelpp-project template application")
set(CPACK_PACKAGE_DESCRIPTION "Cemosis provides a Feel++ template application.")
set(CPACK_PACKAGE_VENDOR "Cemosis")
set(CPACK_PACKAGE_CONTACT "Christophe Prud'homme <christophe.prudhomme@cemosis.fr>")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_GENERATOR "TGZ;DEB")
set(CPACK_SOURCE_GENERATOR "TGZ")

# Debian-specific packaging configuration
set(CPACK_DEBIAN_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_CONTACT}")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${DEBIAN_ARCH}")

# Enhanced dependency specification
if ( USE_FEELPP_TOOLBOXES )
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "libfeelpp1 (>= 0.109), libfeelpp-toolboxes1 (>= 0.109)")
else()
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "libfeelpp1 (>= 0.109)")
endif()

# Add common runtime dependencies
set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libc6, libgcc-s1, libstdc++6")

set(CPACK_DEBIAN_PACKAGE_DESCRIPTION_SUMMARY "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION}")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_SECTION "science")
set(CPACK_DEBIAN_PACKAGE_VERSION "${FULL_PACKAGE_VERSION}")

# Additional Debian package metadata
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/feelpp/feelpp-project")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)  # Automatically detect shared library dependencies

# set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_SOURCE_DIR}/cmake/debian/postinst")
  
include(CPack)

