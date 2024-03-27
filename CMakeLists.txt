

# This is a skeleton CMakeLists.txt file, auto-generated on
# Wed Mar 6 12:36:22 CET 2024.  The developer(s) of this package should delete
# this comment as well as adding dependent targets, packages,
# etc. specific to the package. For details on how to write a package,
# please see
# https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/

cmake_minimum_required(VERSION 3.12)
project(crtmodules VERSION 0.0.0)

find_package(daq-cmake REQUIRED)

daq_setup_environment()


find_package(appfwk REQUIRED)
find_package(logging REQUIRED)
find_package(opmonlib REQUIRED)
find_package(readoutmodules REQUIRED)
find_package(fdreadoutlibs REQUIRED)
find_package(fddetdataformats REQUIRED)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_codegen

daq_codegen(crtreader.jsonnet TEMPLATES Structs.hpp.j2 Nljs.hpp.j2)
daq_codegen(crtcontrollermodule.jsonnet TEMPLATES Structs.hpp.j2 Nljs.hpp.j2)
daq_codegen(crtcontrollermoduleinfo.jsonnet DEP_PKGS opmonlib TEMPLATES opmonlib/InfoStructs.hpp.j2 opmonlib/InfoNljs.hpp.j2)

##############################################################################

set(CRTMODULES_DEPENDENCIES
        #tools
        appfwk::appfwk
        logging::logging
        readoutlibs::readoutlibs
        readoutmodules::readoutmodules
        fdreadoutlibs::fdreadoutlibs
        fddetdataformats::fddetdataformats
)

# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_library

daq_add_library( *.cc LINK_LIBRARIES ${CRTMODULES_DEPENDENCIES} ) # Any source files and/or dependent libraries to link in not yet determined

##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_python_bindings


daq_add_python_bindings(*.cpp LINK_LIBRARIES ${PROJECT_NAME} ) # Any additional libraries to link in beyond the main library not yet determined


##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_plugin

daq_add_plugin(CRTControllerModule duneDAQModule LINK_LIBRARIES appfwk::appfwk) # Replace appfwk library with a more specific library when appropriate
daq_add_plugin(CRTReader duneDAQModule LINK_LIBRARIES crtmodules appfwk::appfwk fdreadoutlibs::fdreadoutlibs readoutmodules::readoutmodules)

##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_application

daq_add_application(startallboards startallboards.cxx LINK_LIBRARIES ) # Any libraries to link in not yet determined

##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_unit_test

daq_add_unit_test(Placeholder_test LINK_LIBRARIES)  # Placeholder_test should be replaced with real unit tests

##############################################################################

daq_install()

