

# This is a skeleton CMakeLists.txt file, auto-generated on
# Wed Mar 6 12:36:22 CET 2024.  The developer(s) of this package should delete
# this comment as well as adding dependent targets, packages,
# etc. specific to the package. For details on how to write a package,
# please see
# https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/

cmake_minimum_required(VERSION 3.12)
project(crtmodules VERSION 2.1.2)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

find_package(appfwk REQUIRED)
find_package(logging REQUIRED)
find_package(opmonlib REQUIRED)
find_package(fdreadoutlibs REQUIRED)
find_package(fddetdataformats REQUIRED)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

daq_protobuf_codegen(opmon/*.proto)

##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_codegen

#daq_codegen(crtcontrollermodule.jsonnet TEMPLATES Structs.hpp.j2 Nljs.hpp.j2)

##############################################################################

set(CRTMODULES_DEPENDENCIES
	#tools
	logging::logging
	fddetdataformats::fddetdataformats
)

# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_library

daq_add_library( *.cc LINK_LIBRARIES ${CRTMODULES_DEPENDENCIES} ) # Any source files and/or dependent libraries to link in not yet determined

##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_python_bindings


daq_add_python_bindings(*.cpp )


##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_plugin

#daq_add_plugin(CRTControllerModule duneDAQModule LINK_LIBRARIES crtmodules appfwk::appfwk) # Replace appfwk library with a more specific library when appropriate
daq_add_plugin(CRTGrenobleReaderModule duneDAQModule LINK_LIBRARIES crtmodules appfwk::appfwk fdreadoutlibs::fdreadoutlibs opmonlib::opmonlib)
daq_add_plugin(CRTBernReaderModule duneDAQModule LINK_LIBRARIES crtmodules appfwk::appfwk fdreadoutlibs::fdreadoutlibs opmonlib::opmonlib)

##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_application
SET_SOURCE_FILES_PROPERTIES( main.c PROPERTIES COMPILE_FLAGS "-Wno-pointer-sign -Wno-unused-parameter -Wno-unused-label -Wno-unused-but-set-variable -Wno-unused-function -Wno-return-type -Wno-format")
#daq_add_application(crt_readout main.c LINK_LIBRARIES -lusb-1.0 -lm -lrt) # Any libraries to link in not yet determined
#daq_add_application(startallboards startallboards.cxx LINK_LIBRARIES ) # Any libraries to link in not yet determined

##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_unit_test

# daq_add_unit_test(Placeholder_test LINK_LIBRARIES)  # Placeholder_test should be replaced with real unit tests

##############################################################################

daq_install()

