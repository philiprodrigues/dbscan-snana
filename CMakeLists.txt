cmake_minimum_required(VERSION 3.1)

project(dbscan-snana VERSION 0.1
  LANGUAGES CXX)

find_package(ROOT REQUIRED)

add_subdirectory(cnpy)

add_executable(extract-snana-hits extract-snana-hits.cpp)
target_link_libraries(extract-snana-hits PUBLIC ROOT::Tree ROOT::TreePlayer cnpy)
install(TARGETS extract-snana-hits)


