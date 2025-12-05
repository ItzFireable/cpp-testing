cmake_minimum_required(VERSION 3.20)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB SOURCES CONFIGURE_DEPENDS 
    "src/*.cpp"
    "src/utils/*.cpp"
    "src/objects/*.cpp"
    "src/states/*.cpp"
    "src/rhythm/*.cpp"
)

option(USE_VENDORED "Use vendored libraries" ON)
option(USE_CONSOLE "Use console application" OFF)

project(game-testing)
add_executable(app ${SOURCES})

target_compile_features(app PRIVATE cxx_std_17)

target_compile_options(app PRIVATE 
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++17>
    -pthread
)

list(APPEND SOURCES "${CMAKE_SOURCE_DIR}/src/utils/stb_vorbis_impl.cpp")

if(MSVC)
    target_compile_options(app PRIVATE /EHa)
endif()

target_include_directories(app PRIVATE vendored/include)
target_include_directories(app PRIVATE include)

if(USE_VENDORED)
    set(FREEIMAGE_DIR "${CMAKE_SOURCE_DIR}/vendored/FreeImage")
    set(GLEW_DIR "${CMAKE_SOURCE_DIR}/vendored/glew")

    if(WIN32 AND CMAKE_GENERATOR MATCHES "MinGW")
        set(MAKE_CMD "mingw32-make")
    elseif(WIN32 AND CMAKE_GENERATOR MATCHES "NMake")
        set(MAKE_CMD "nmake")
    else()
        set(MAKE_CMD "make")
    endif()

    add_custom_target(FreeImage_Build
        COMMAND ${MAKE_CMD}
        WORKING_DIRECTORY ${FREEIMAGE_DIR}
        COMMENT "Building FreeImage static library"
        USES_TERMINAL
    )

    add_custom_target(GLEW_Build
        COMMAND ${MAKE_CMD} all 
        WORKING_DIRECTORY ${GLEW_DIR}
        COMMENT "Building GLEW static library"
        USES_TERMINAL
    )

    if(WIN32)
        set(FREEIMAGE_LIB_PATH "${FREEIMAGE_DIR}/Dist/FreeImage.lib")
        set(GLEW_LIB_PATH "${GLEW_DIR}/lib/Release/x64/glew32s.lib")
        target_compile_definitions(app PRIVATE GLEW_STATIC)
    else()
        set(FREEIMAGE_LIB_PATH "${FREEIMAGE_DIR}/libfreeimage.a")
        set(GLEW_LIB_PATH "${GLEW_DIR}/lib/libglew.a")
    endif()
    
    add_library(FreeImage STATIC IMPORTED)
    set_property(TARGET FreeImage PROPERTY IMPORTED_LOCATION ${FREEIMAGE_LIB_PATH})

    add_library(GLEW::GLEW STATIC IMPORTED)
    set_property(TARGET GLEW::GLEW PROPERTY IMPORTED_LOCATION ${GLEW_LIB_PATH})
    
    set_property(TARGET FreeImage PROPERTY INTERFACE_INCLUDE_DIRECTORIES 
        ${FREEIMAGE_DIR}/Source 
        ${FREEIMAGE_DIR}/Wrapper/FreeImagePlus
    )

    set_property(TARGET GLEW::GLEW PROPERTY INTERFACE_INCLUDE_DIRECTORIES 
        ${GLEW_DIR}/include 
    )

    add_dependencies(app FreeImage_Build)
    add_dependencies(app GLEW_Build)
    target_link_libraries(app PRIVATE FreeImage GLEW::GLEW)

    add_subdirectory(vendored/glfw EXCLUDE_FROM_ALL)
    add_subdirectory(vendored/freetype EXCLUDE_FROM_ALL)
    
    find_library(OPENAL_LIBRARY 
        NAMES OpenAL32 
        PATHS ${CMAKE_SOURCE_DIR}/vendored/openal-soft
        NO_DEFAULT_PATH
    )
    
    if(NOT OPENAL_LIBRARY)
        message(WARNING "Using vendored OpenAL source build (may have CRT issues)")
        add_subdirectory(vendored/openal-soft EXCLUDE_FROM_ALL)
        set(OPENAL_TARGET OpenAL)
    else()
        message(STATUS "Using prebuilt OpenAL from: ${OPENAL_LIBRARY}")
        add_library(OpenAL_prebuilt SHARED IMPORTED)
        set_target_properties(OpenAL_prebuilt PROPERTIES
            IMPORTED_LOCATION ${OPENAL_LIBRARY}
            IMPORTED_IMPLIB ${OPENAL_LIBRARY}
        )
        set(OPENAL_TARGET OpenAL_prebuilt)
    endif()

    target_include_directories(app PRIVATE 
        vendored/glfw/include
        vendored/glm
        vendored/freetype/include
        vendored/FreeImage/Source
        vendored/openal-soft/include
    )

    target_link_libraries(app PRIVATE 
        glfw
        glm
        freetype
    )

    find_package(OpenGL REQUIRED)
    target_link_libraries(app PRIVATE OpenGL::GL)
    
    target_link_libraries(app PRIVATE ${OPENAL_TARGET})
    add_custom_command(TARGET app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:OpenAL>
        $<TARGET_FILE_DIR:app>
        COMMENT "Copying OpenAL32.dll to executable directory"
    )

else()
    find_package(glfw3 REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(glm REQUIRED)
    find_package(Freetype REQUIRED)
    find_package(FreeImage REQUIRED)
    find_package(OpenGL REQUIRED)
    find_package(OpenAL REQUIRED)

    target_include_directories(app PRIVATE 
        ${GLEW_INCLUDE_DIRS}
        ${FREETYPE_INCLUDE_DIRS}
        ${FREEIMAGE_INCLUDE_DIRS}
    )

    target_link_libraries(app PRIVATE 
        glfw
        GLEW::GLEW
        glm::glm
        Freetype::Freetype
        FreeImage::FreeImage
        OpenGL::GL
        OpenAL::OpenAL
    )
endif()

# Copy assets directory
add_custom_command(TARGET app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:app>/assets
)

target_compile_definitions(app PRIVATE SDL_MAIN_USE_CALLBACKS)
target_link_libraries(app PRIVATE 
    pthread
)

if(WIN32)
    target_link_libraries(app PRIVATE opengl32)
elseif(APPLE)
    find_library(OPENGL_LIBRARY OpenGL)
    target_link_libraries(app PRIVATE ${OPENGL_LIBRARY})
else()
    target_link_libraries(app PRIVATE GL)
endif()


message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Executable will be in: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(STATUS "Building 64-bit executable")
else()
    message(STATUS "Building 32-bit executable")
endif()