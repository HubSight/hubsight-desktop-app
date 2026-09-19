cmake_minimum_required(VERSION 3.21)

project(HubSight
    VERSION 0.1.0
    DESCRIPTION "HubSight desktop application"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Network Qml Quick QuickControls2 QuickDialogs2 LinguistTools)

set(HUBSIGHT_SDK_PREFIX "" CACHE PATH
    "Installation prefix containing HubSight Admin SDK and Preferences configs")
set(HUBSIGHT_SDK_SOURCE_DIR "" CACHE PATH
    "Local HubSight qt-sdk source checkout")
option(HUBSIGHT_FETCH_SDK
       "Fetch HubSight qt-sdk v0.2.0 from GitHub when no local SDK is supplied"
       OFF)

set(HUBSIGHT_SDK_IN_TREE OFF)
if (HUBSIGHT_SDK_SOURCE_DIR)
    if (NOT EXISTS "${HUBSIGHT_SDK_SOURCE_DIR}/CMakeLists.txt")
        message(FATAL_ERROR
            "HUBSIGHT_SDK_SOURCE_DIR does not contain a CMake project: "
            "${HUBSIGHT_SDK_SOURCE_DIR}")
    endif ()
    set(HUBSIGHT_ADMIN_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(HUBSIGHT_ADMIN_BUILD_INTEGRATION_TESTS OFF CACHE BOOL "" FORCE)
    add_subdirectory("${HUBSIGHT_SDK_SOURCE_DIR}"
                     "${CMAKE_BINARY_DIR}/hubsight-qt-sdk")
    set(HUBSIGHT_SDK_IN_TREE ON)
elseif (HUBSIGHT_FETCH_SDK)
    include(FetchContent)
    set(HUBSIGHT_ADMIN_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(HUBSIGHT_ADMIN_BUILD_INTEGRATION_TESTS OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(hubsight_qt_sdk
        GIT_REPOSITORY https://github.com/HubSight/qt-sdk.git
        GIT_TAG v0.2.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(hubsight_qt_sdk)
    set(HUBSIGHT_SDK_IN_TREE ON)
endif ()

# Make the sibling checkout convenient for the current workspace while still
# allowing a normal installed SDK through -DHUBSIGHT_SDK_PREFIX=... .
if (NOT HUBSIGHT_SDK_PREFIX)
    file(GLOB _hubsight_sdk_candidates
        LIST_DIRECTORIES true
        "${CMAKE_CURRENT_SOURCE_DIR}/../qt-sdk/dist/hubsight-admin-sdk-*")
    list(SORT _hubsight_sdk_candidates COMPARE NATURAL ORDER DESCENDING)
    foreach (_candidate IN LISTS _hubsight_sdk_candidates)
        if (EXISTS "${_candidate}/lib/cmake/HubSightAdminSdk/HubSightAdminSdkConfig.cmake")
            set(HUBSIGHT_SDK_PREFIX "${_candidate}")
            break()
        endif ()
    endforeach ()
endif ()

if (HUBSIGHT_SDK_PREFIX AND NOT HUBSIGHT_SDK_IN_TREE)
    list(PREPEND CMAKE_PREFIX_PATH "${HUBSIGHT_SDK_PREFIX}")
endif ()

if (NOT HUBSIGHT_SDK_IN_TREE)
    find_package(HubSightAdminSdk 0.2 CONFIG REQUIRED)
    find_package(HubSightPreferences 0.2 CONFIG REQUIRED)
endif ()

set(HUBSIGHT_SOURCES
    src/main.cpp
    src/appcontroller.cpp
    src/appcontroller.hpp
    src/resources.qrc
)

set(HUBSIGHT_QML_SOURCES
    qml/Main.qml
    qml/ImportWizard.qml
    qml/AuthPage.qml
    qml/Workspace.qml
    qml/Splash.qml
    qml/components/AppButton.qml
    qml/components/AppCard.qml
    qml/components/AppTextField.qml
    qml/components/FeatureCard.qml
    qml/components/NavButton.qml
)

set(HUBSIGHT_TRANSLATIONS
    translations/hubsight_vi.ts
)

include(GNUInstallDirs)

if (APPLE)
    set(HUBSIGHT_MACOS_INFO_PLIST
        "${CMAKE_CURRENT_BINARY_DIR}/HubSight-Info.plist")
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/macos/Info.plist.in"
        "${HUBSIGHT_MACOS_INFO_PLIST}"
        @ONLY)
    list(APPEND HUBSIGHT_SOURCES assets/icons/macos/hubsight.icns)
    set_source_files_properties(assets/icons/macos/hubsight.icns PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources")
endif ()

if (WIN32)
    set(HUBSIGHT_WINDOWS_ASSOCIATION_SCRIPT
        "${CMAKE_CURRENT_BINARY_DIR}/register-hscfg-association.cmake")
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/windows/register-hscfg-association.cmake.in"
        "${HUBSIGHT_WINDOWS_ASSOCIATION_SCRIPT}"
        @ONLY)
    list(APPEND HUBSIGHT_SOURCES src/hubsight.rc)
endif ()

add_executable(HubSight
    ${HUBSIGHT_SOURCES}
    ${HUBSIGHT_QML_SOURCES}
    ${HUBSIGHT_TRANSLATIONS}
)

qt_add_translations(HubSight
    TS_FILES ${HUBSIGHT_TRANSLATIONS}
    RESOURCE_PREFIX "/i18n"
)

target_link_libraries(HubSight
    PRIVATE
        Qt6::Gui
        Qt6::Network
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::QuickDialogs2
        HubSight::AdminSdk
        HubSight::Preferences
)

target_include_directories(HubSight
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if (APPLE)
    set_target_properties(HubSight PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_ICON_FILE "hubsight.icns"
        MACOSX_BUNDLE_BUNDLE_NAME "HubSight"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.hubsight.desktop"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_INFO_PLIST "${HUBSIGHT_MACOS_INFO_PLIST}"
    )
elseif (WIN32)
    set_target_properties(HubSight PROPERTIES WIN32_EXECUTABLE TRUE)
endif ()

install(TARGETS HubSight
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if (UNIX AND NOT APPLE)
    install(FILES assets/icons/linux/hubsight.svg
        DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps"
        RENAME hubsight.svg)
    install(FILES assets/icons/linux/hubsight-512x512.png
        DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/512x512/apps"
        RENAME hubsight.png)
    install(FILES assets/linux/com.hubsight.desktop
        DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")
    install(FILES assets/linux/hubsight.xml
        DESTINATION "${CMAKE_INSTALL_DATADIR}/mime/packages")
    install(CODE [[
        find_program(HUBSIGHT_UPDATE_MIME_DATABASE update-mime-database)
        if (HUBSIGHT_UPDATE_MIME_DATABASE)
            execute_process(
                COMMAND "${HUBSIGHT_UPDATE_MIME_DATABASE}"
                        "${CMAKE_INSTALL_PREFIX}/share/mime")
        endif ()
        find_program(HUBSIGHT_UPDATE_DESKTOP_DATABASE update-desktop-database)
        if (HUBSIGHT_UPDATE_DESKTOP_DATABASE)
            execute_process(
                COMMAND "${HUBSIGHT_UPDATE_DESKTOP_DATABASE}"
                        "${CMAKE_INSTALL_PREFIX}/share/applications")
        endif ()
    ]])
endif ()

if (WIN32)
    install(SCRIPT "${HUBSIGHT_WINDOWS_ASSOCIATION_SCRIPT}")
endif ()
