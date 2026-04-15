cmake_minimum_required(VERSION 3.16)
project(AsyncSql VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find required Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Gui Sql Widgets)

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

# Example application sources
set(APP_SOURCES
    examples/databaseviewer/main.cpp
    examples/databaseviewer/mainwindow.cpp
    examples/databaseviewer/optionsdialog.cpp
)

set(APP_HEADERS
    examples/databaseviewer/mainwindow.h
    examples/databaseviewer/optionsdialog.h
)

set(APP_FORMS
    examples/databaseviewer/mainwindow.ui
    examples/databaseviewer/optionsdialog.ui
)

# Build executable
add_executable(AsyncSql
    ${LIB_SOURCES}
    ${LIB_HEADERS}
    ${APP_SOURCES}
    ${APP_HEADERS}
    ${APP_FORMS}
)

target_link_libraries(AsyncSql PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Sql
    Qt6::Widgets
)

target_include_directories(AsyncSql PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/asyncsql
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/databaseviewer
)

# Install rules
install(TARGETS AsyncSql
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
    TARGET AsyncSql
    OUTPUT_SCRIPT deploy_script
)
install(SCRIPT ${deploy_script})
