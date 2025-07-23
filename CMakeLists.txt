cmake_minimum_required(VERSION 3.15)

project(Scrollmore LANGUAGES CXX)

find_package(ftxui CONFIG REQUIRED)
find_package(cxxopts CONFIG REQUIRED)

add_executable(scroller src/scroller.cpp)
set_target_properties(scroller PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin/Debug"
  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/Release"
  RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/bin/RelWithDebInfo"
  RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/bin/MinSizeRel"
)
target_link_libraries(scroller PRIVATE ftxui::dom ftxui::screen ftxui::component)
target_compile_features(scroller PRIVATE cxx_std_20)

add_executable(more src/more.cpp)
set_target_properties(more PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin/Debug"
  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/Release"
  RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/bin/RelWithDebInfo"
  RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/bin/MinSizeRel"
)
target_link_libraries(more PRIVATE ftxui::dom ftxui::screen ftxui::component cxxopts::cxxopts)
target_compile_features(more PRIVATE cxx_std_20)

# Copy input to all output directories after build
foreach(cfg IN ITEMS Debug Release RelWithDebInfo MinSizeRel)
    add_custom_command(TARGET more POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/input"
            "${CMAKE_SOURCE_DIR}/bin/${cfg}/input"
    )
    add_custom_command(TARGET scroller POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/input"
            "${CMAKE_SOURCE_DIR}/bin/${cfg}/input"
    )
endforeach()
