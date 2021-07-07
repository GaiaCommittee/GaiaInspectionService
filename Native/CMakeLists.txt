#==============================
# Requirements
#==============================

cmake_minimum_required(VERSION 3.10)

#==============================
# Project Settings
#==============================

if (NOT PROJECT_DECLARED)
    project("Gaia Inspection Service" LANGUAGES CXX VERSION 0.8)
    set(PROJECT_DECLARED)
endif()

#==============================
# Project Modules
#==============================

add_subdirectory("GaiaInspectionClient")

if (WITH_TEST)
    add_subdirectory("InspectionTest")
endif()
