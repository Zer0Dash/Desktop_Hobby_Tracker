cmake_minimum_required(VERSION 3.19)
project(Gui_Hobby_Tracker LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Charts Network)


qt_standard_project_setup()

qt_add_executable(Gui_Hobby_Tracker
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    loginwindow.h
    loginwindow.cpp
    apimanager.h
    apimanager.cpp
    registerwindow.h
    registerwindow.cpp

)

target_link_libraries(Gui_Hobby_Tracker
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Charts
        Qt6::Network
)

install(TARGETS Gui_Hobby_Tracker
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET Gui_Hobby_Tracker
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
