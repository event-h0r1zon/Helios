cmake_minimum_required(VERSION 3.20)

project(Helios LANGUAGES CXX)

add_executable(helios 
  app/main.cpp
  src/Spacecraft.cpp
  src/Propagator.cpp
)

find_package(Eigen3 CONFIG REQUIRED)
target_link_libraries(helios PRIVATE Eigen3::Eigen)

target_compile_features(helios PRIVATE cxx_std_20)
target_include_directories(helios PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(helios PROPERTIES
  CXX_EXTENSIONS OFF
)