
# ==============================================

cmake_minimum_required(VERSION 3.22)

# ==============================================

add_subdirectory(JUCE)
add_subdirectory(libs)

# ==============================================

set(LIBRARY_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/basic_json/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/basic_simd/include")

# ==============================================

set(GENERATED_SOURCES_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(GENERATED_SOURCES_DIR "${GENERATED_SOURCES_DIR}" PARENT_SCOPE)

set(PROJECT_SOURCE_DIRECTORY "${CMAKE_SOURCE_DIR}")
set(PROJECT_SOURCE_DIRECTORY "${PROJECT_SOURCE_DIRECTORY}" PARENT_SCOPE)
set(CORE_SOURCE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
set(CORE_SOURCE_DIRECTORY "${CORE_SOURCE_DIRECTORY}" PARENT_SCOPE)

file(GLOB_RECURSE PROJECT_SOURCE
    "${PROJECT_SOURCE_DIRECTORY}/source/*.cpp"
    "${PROJECT_SOURCE_DIRECTORY}/include/*.hpp"
    "${PROJECT_SOURCE_DIRECTORY}/resources/*.xml"
    "${PROJECT_SOURCE_DIRECTORY}/theme/*.json"
    "${GENERATED_SOURCES_DIR}/*.json"
    "${GENERATED_SOURCES_DIR}/*.hpp"
    "${GENERATED_SOURCES_DIR}/*.xml"
)

file(GLOB_RECURSE CORE_SOURCE
    "${CORE_SOURCE_DIRECTORY}/source/*.cpp"
    "${CORE_SOURCE_DIRECTORY}/include/*.hpp"
    "${CORE_SOURCE_DIRECTORY}/resources/*.xml"
)

source_group(TREE ${PROJECT_SOURCE_DIRECTORY} FILES ${PROJECT_SOURCE})
source_group(TREE ${CORE_SOURCE_DIRECTORY} FILES ${CORE_SOURCE})

# ==============================================

if(${VERSION_TYPE} STREQUAL "RELEASE")
    set(FULL_VERSION "${VERSION}")
else()
    set(FULL_VERSION "${VERSION}-${VERSION_TYPE}")
endif()

add_compile_definitions(
    SYNTH_InitialSize=${INITIAL_SIZE}
    SYNTH_VersionType="${VERSION_TYPE}"
    SYNTH_FullVersion="${FULL_VERSION}"
)

juce_add_plugin(${NAME}
    VERSION ${VERSION}
    COMPANY_NAME ${COMPANY}
    COMPANY_WEBSITE ${WEBSITE}
    BUNDLE_ID ${BUNDLE_IDENTIFIER}
    PLUGIN_MANUFACTURER_CODE ${COMPANY_CODE}
    PLUGIN_CODE ${PLUGIN_CODE}
    IS_SYNTH ${IS_SYNTH}
    NEEDS_MIDI_INPUT ${IS_SYNTH}
    FORMATS AU VST3 Standalone)

juce_generate_juce_header(${NAME})

target_sources(${NAME} 
    PRIVATE
        ${PROJECT_SOURCE}
        ${CORE_SOURCE})

target_include_directories(${NAME} 
    PRIVATE 
        "${CORE_SOURCE_DIRECTORY}/include/"
        "${PROJECT_SOURCE_DIRECTORY}/include/"
        ${GENERATED_SOURCES_DIR}
        ${LIBRARY_INCLUDE_DIRS})

target_compile_definitions(${NAME}
    PUBLIC
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_USE_MP3AUDIOFORMAT=1
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(${NAME}
    PRIVATE
        juce::juce_core
        juce::juce_audio_utils
        juce::juce_gui_extra
        juce::juce_opengl
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

target_precompile_headers(${NAME} PRIVATE "${CORE_SOURCE_DIRECTORY}/include/Kaixo/Core/pch.hpp")

# ==============================================

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
)
FetchContent_MakeAvailable(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)

enable_testing()

file(GLOB_RECURSE TESTS_SOURCE
    "${CORE_SOURCE_DIRECTORY}/tests/source/*.cpp"
    "${CORE_SOURCE_DIRECTORY}/tests/include/*.hpp"
    "${PROJECT_SOURCE_DIRECTORY}/tests/source/*.cpp"
    "${PROJECT_SOURCE_DIRECTORY}/tests/include/*.hpp"
)

source_group(TREE ${CORE_SOURCE_DIRECTORY} FILES ${TESTS_SOURCE})

add_executable(UnitTests
    ${TESTS_SOURCE})

add_dependencies(UnitTests gtest)

get_target_property(${NAME}_INCLUDES ${NAME} INCLUDE_DIRECTORIES)
get_target_property(${NAME}_LIBRARIES ${NAME} LINK_LIBRARIES)
get_target_property(${NAME}_DEFINITIONS ${NAME} COMPILE_DEFINITIONS)

target_include_directories(UnitTests
    PRIVATE
        ${CORE_SOURCE_DIRECTORY}/tests/include
        ${PROJECT_SOURCE_DIRECTORY}/tests/include
        "${${NAME}_INCLUDES}")

target_compile_definitions(UnitTests
    PRIVATE
        "${${NAME}_DEFINITIONS}")
    
target_link_libraries(UnitTests 
    "${NAME}"
    "${${NAME}_LIBRARIES}"
    GTest::GTest)

target_precompile_headers(UnitTests PRIVATE "${CORE_SOURCE_DIRECTORY}/tests/include/Kaixo/Test/pch.hpp")

add_test(UnitTests_gtests UnitTests)

# ==============================================

file(GLOB_RECURSE GENERATOR_SOURCE
    "${CORE_SOURCE_DIRECTORY}/generators/*.cpp"
    "${CORE_SOURCE_DIRECTORY}/generators/*.hpp"
)

source_group(TREE ${CORE_SOURCE_DIRECTORY} FILES ${GENERATOR_SOURCE})
    
add_executable(Generator
    ${GENERATOR_SOURCE})

target_include_directories(Generator
    PRIVATE
        ${CORE_SOURCE_DIRECTORY}/include/
        ${CORE_SOURCE_DIRECTORY}/generators/
        ${LIBRARY_INCLUDE_DIRS})

# ==============================================

set(GENERATED_PARAMETERS "Parameters.hpp")
set(GENERATED_ASSIGNERS "Assigners.hpp") 

set(GENERATE_PARAMETERS_FROM ${PROJECT_SOURCE_DIRECTORY}/${PARAMETERS})
set(GENERATE_PARAMETERS_TO ${GENERATED_SOURCES_DIR}/${GENERATED_PARAMETERS})
set(GENERATE_ASSIGNERS_TO ${GENERATED_SOURCES_DIR}/${GENERATED_ASSIGNERS})

set(GENERATED_DEFAULT_THEME "DefaultTheme.hpp")

set(GENERATE_DEFAULT_THEME_FROM ${PROJECT_SOURCE_DIRECTORY}/${THEME})
set(GENERATE_DEFAULT_THEME_TO ${GENERATED_SOURCES_DIR}/${GENERATED_DEFAULT_THEME})

set(GENERATED_THEME "GeneratedTheme.hpp")
set(GENERATED_THEME_SCHEMA "schema.json")

set(GENERATE_THEME_FROM ${PROJECT_SOURCE_DIRECTORY}/${THEME_SCHEMA})
set(GENERATE_THEME_SCHEMA_TO ${GENERATED_SOURCES_DIR}/${GENERATED_THEME_SCHEMA})
set(GENERATE_THEME_TO ${GENERATED_SOURCES_DIR}/${GENERATED_THEME})

add_custom_command(OUTPUT ${GENERATE_PARAMETERS_TO}
    COMMAND Generator "parameters" ${GENERATE_PARAMETERS_FROM} ${GENERATE_PARAMETERS_TO} ${GENERATE_ASSIGNERS_TO}
    DEPENDS ${GENERATE_PARAMETERS_FROM}
)

add_custom_command(OUTPUT ${GENERATE_DEFAULT_THEME_TO}
    COMMAND Generator "theme" ${GENERATE_DEFAULT_THEME_FROM} ${GENERATE_DEFAULT_THEME_TO}
    DEPENDS ${GENERATE_DEFAULT_THEME_FROM}
)

add_custom_command(OUTPUT ${GENERATE_THEME_TO}
    COMMAND Generator "schema" ${GENERATE_THEME_FROM} ${GENERATE_THEME_SCHEMA_TO} ${GENERATE_THEME_TO}
    DEPENDS ${GENERATE_THEME_FROM}
)

add_custom_target(GenerateParameters
    DEPENDS ${GENERATE_PARAMETERS_TO} ${GENERATE_ASSIGNERS_TO}
)

add_custom_target(GenerateTheme
    DEPENDS ${GENERATE_DEFAULT_THEME_TO}
)

add_custom_target(GenerateSchema
    DEPENDS ${GENERATE_THEME_TO} ${GENERATE_THEME_SCHEMA_TO}
)

add_dependencies(${NAME} GenerateParameters GenerateTheme GenerateSchema)

# ==============================================
