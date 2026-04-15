cmake_minimum_required(VERSION 3.16)
project(AsyncSql VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find required Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Gui Sql Widgets)

# Try to find QML/Quick components (optional, for QML examples)
find_package(Qt6 COMPONENTS Qml Quick)

qt_standard_project_setup()

# AsyncSql library sources
set(LIB_SOURCES
    asyncsql/asyncsqltablemodel.cpp
    asyncsql/asyncmodelregister.cpp
    asyncsql/databaseconnection.cpp
    asyncsql/databaseexception.cpp
    asyncsql/queryrequest.cpp
    asyncsql/queryresult.cpp
    asyncsql/querythread.cpp
    asyncsql/queryworker.cpp
)

set(LIB_HEADERS
    asyncsql/asyncsqltablemodel.h
    asyncsql/asyncmodelregister.h
    asyncsql/databaseconnection.h
    asyncsql/databaseexception.h
    asyncsql/queryrequest.h
    asyncsql/queryresult.h
    asyncsql/querythread.h
    asyncsql/queryworker.h
)

# ---- Widgets Example App ----

set(WIDGETS_APP_SOURCES
    examples/databaseviewer/main.cpp
    examples/databaseviewer/mainwindow.cpp
    examples/databaseviewer/optionsdialog.cpp
)

set(WIDGETS_APP_HEADERS
    examples/databaseviewer/mainwindow.h
    examples/databaseviewer/optionsdialog.h
)

set(WIDGETS_APP_FORMS
    examples/databaseviewer/mainwindow.ui
    examples/databaseviewer/optionsdialog.ui
)

add_executable(AsyncSqlViewer
    ${LIB_SOURCES}
    ${LIB_HEADERS}
    ${WIDGETS_APP_SOURCES}
    ${WIDGETS_APP_HEADERS}
    ${WIDGETS_APP_FORMS}
)

target_link_libraries(AsyncSqlViewer PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Sql
    Qt6::Widgets
)

target_include_directories(AsyncSqlViewer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/asyncsql
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/databaseviewer
)

# ---- QML Example App (only if QML/Quick are available) ----

if(Qt6Qml_FOUND AND Qt6Quick_FOUND)
    set(QML_APP_SOURCES
        examples/qmlviewer/main.cpp
    )

    set(QML_RESOURCES
        examples/qmlviewer/qml.qrc
    )

    add_executable(AsyncSqlQml
        ${LIB_SOURCES}
        ${LIB_HEADERS}
        ${QML_APP_SOURCES}
        ${QML_RESOURCES}
    )

    target_link_libraries(AsyncSqlQml PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Sql
        Qt6::Widgets
        Qt6::Qml
        Qt6::Quick
    )

    target_include_directories(AsyncSqlQml PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/asyncsql
    )

    set_target_properties(AsyncSqlQml PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER com.asyncsql.qmlviewer
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
    )

    qt_generate_deploy_app_script(
        TARGET AsyncSqlQml
        OUTPUT_SCRIPT qml_deploy_script
    )
    install(SCRIPT ${qml_deploy_script})
endif()

# ---- Install rules ----

install(TARGETS AsyncSqlViewer
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
    TARGET AsyncSqlViewer
    OUTPUT_SCRIPT deploy_script
)
install(SCRIPT ${deploy_script})
