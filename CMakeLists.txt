cmake_minimum_required(VERSION 3.16)

project(qt_mpv VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

set(TS_FILES qt_mpv_zh_CN.ts)

# build mpv project
find_program(MESON_EXECUTABLE meson REQUIRED)
find_program(NINJA_EXECUTABLE ninja REQUIRED)

set(MPV_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mpv)
set(MPV_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/mpv-build)
set(MPV_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/mpv-install)

set(MPV_BUILD_OPTIONS
    -Dlibmpv=true
    -Dcplayer=false
    -Dbuild-date=false
    -Dmanpage-build=disabled
    -Dpdf-build=disabled
    -Dlua=disabled
    -Djavascript=disabled
    -Dlibarchive=disabled
    -Dlibbluray=disabled
    -Ddvdnav=disabled
    -Duchardet=disabled
    -Drubberband=disabled
    -Dlcms2=disabled
    -Dopenal=disabled
    -Dspirv-cross=disabled
    -Dvulkan=disabled
    -Dshaderc=disabled
    -Dgl=enabled
    -Dvdpau=disabled
    -Dvaapi=disabled
    -Dwayland=disabled
    -Dxv=disabled
    -Dsdl2=disabled
    -Degl-angle=disabled
    -Degl-angle-win32=disabled
    -Dplain-gl=enabled
    -Dtests=false
)

add_custom_command(
    OUTPUT ${MPV_BUILD_DIR}/build.ninja
    COMMAND ${MESON_EXECUTABLE} setup ${MPV_BUILD_DIR} ${MPV_SOURCE_DIR}
            --prefix=${MPV_INSTALL_DIR}
            ${MPV_BUILD_OPTIONS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Configuring mpv with Meson..."
)

add_custom_command(
    OUTPUT ${MPV_INSTALL_DIR}/lib/libmpv.a
    COMMAND ${NINJA_EXECUTABLE} -C ${MPV_BUILD_DIR} install
    DEPENDS ${MPV_BUILD_DIR}/build.ninja
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building and installing mpv..."
)

add_library(mpv::libmpv STATIC IMPORTED)
set_target_properties(mpv::libmpv PROPERTIES
    IMPORTED_LOCATION ${MPV_INSTALL_DIR}/lib/libmpv.a
)

target_include_directories(mpv::libmpv INTERFACE
    ${MPV_SOURCE_DIR}
    ${MPV_SOURCE_DIR}/include
    ${MPV_BUILD_DIR}
)

add_custom_target(mpv_build ALL
    DEPENDS ${MPV_INSTALL_DIR}/lib/libmpv.a
)

add_dependencies(mpv::libmpv mpv_build)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mpvwidget.h
        mpvwidget.cpp
        mainwindow.ui
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(qt_mpv
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET qt_mpv APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(qt_mpv SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(qt_mpv
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

find_package(Qt6 COMPONENTS
    Core
    Gui
    Widgets
    OpenGLWidgets
    LinguistTools
    REQUIRED
)

target_link_libraries(qt_mpv PRIVATE 
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::OpenGLWidgets
        opengl32)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.qt_mpv)
endif()
set_target_properties(qt_mpv PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS qt_mpv
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(qt_mpv)
endif()
