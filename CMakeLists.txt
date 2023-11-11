
# ==============================================

cmake_minimum_required(VERSION 3.15)

# ==============================================

add_subdirectory(JUCE)

# ==============================================

set(GENERATED_SOURCES_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

set(PROJECT_SOURCE_DIRECTORY "${CMAKE_SOURCE_DIR}")
set(CORE_SOURCE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE PROJECT_SOURCE
    "${PROJECT_SOURCE_DIRECTORY}/source/*.cpp"
    "${PROJECT_SOURCE_DIRECTORY}/include/*.hpp"
    "${PROJECT_SOURCE_DIRECTORY}/resources/*.xaml"
)

file(GLOB_RECURSE CORE_SOURCE
    "${CORE_SOURCE_DIRECTORY}/source/*.cpp"
    "${CORE_SOURCE_DIRECTORY}/include/*.hpp"
)

source_group(TREE ${PROJECT_SOURCE_DIRECTORY} FILES ${PROJECT_SOURCE})
source_group(TREE ${CORE_SOURCE_DIRECTORY} FILES ${CORE_SOURCE})

# ==============================================

add_compile_definitions(
    SYNTH_InitialSize=${INITIAL_SIZE}
    SYNTH_ThemeFile="${PROJECT_SOURCE_DIRECTORY}/${THEME}"
)

juce_add_plugin(${NAME}
    VERSION ${VERSION}
    COMPANY_NAME ${COMPANY}
    COMPANY_WEBSITE ${WEBSITE}
    BUNDLE_ID ${BUNDLE_IDENTIFIER}
    PLUGIN_MANUFACTURER_CODE ${COMPANY_CODE}
    PLUGIN_CODE ${PLUGIN_CODE}
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
        ${GENERATED_SOURCES_DIR})

target_compile_definitions(${NAME}
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(${NAME}
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# ==============================================

file(GLOB_RECURSE GENERATOR_SOURCE
    "${CORE_SOURCE_DIRECTORY}/generators/*.cpp"
    "${CORE_SOURCE_DIRECTORY}/generators/*.hpp"
)

source_group(TREE ${CORE_SOURCE_DIRECTORY} FILES ${GENERATOR_SOURCE})
    
add_executable(GenerateParameters
    ${GENERATOR_SOURCE})

target_include_directories(GenerateParameters
    PRIVATE
        ${CORE_SOURCE_DIRECTORY}generators/)

# ==============================================

set(GENERATED_PARAMETERS "Parameters.hpp")
set(GENERATED_ASSIGNERS "Assigners.hpp") 
set(GENERATE_PARAMETERS_FROM ${CMAKE_SOURCE_DIR}/${PARAMETERS})
set(GENERATE_PARAMETERS_TO ${GENERATED_SOURCES_DIR}/${GENERATED_PARAMETERS})
set(GENERATE_ASSIGNERS_TO ${GENERATED_SOURCES_DIR}/${GENERATED_ASSIGNERS})

add_custom_command(OUTPUT ${GENERATE_PARAMETERS_TO}
    COMMAND GenerateParameters ${GENERATE_PARAMETERS_FROM} ${GENERATE_PARAMETERS_TO} ${GENERATE_ASSIGNERS_TO}
    DEPENDS ${GENERATE_PARAMETERS_FROM}
)

add_custom_target(GenerateParameters_run
    DEPENDS ${GENERATE_PARAMETERS_TO} ${GENERATE_ASSIGNERS_TO}
)

add_dependencies(${NAME} GenerateParameters_run)

# ==============================================
