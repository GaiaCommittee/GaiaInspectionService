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

if (NOT PROJECT_SUIT)
    set(PROJECT_SUIT TRUE)
endif()

#==============================
# Project Modules
#==============================

add_subdirectory("GaiaInspectionClient")
add_subdirectory("GaiaInspectionReader")
add_subdirectory("GaiaInspectionWatcher")
add_subdirectory("GaiaInspectionChart")

if (WITH_TEST)
    add_subdirectory("InspectionTest")
endif()
