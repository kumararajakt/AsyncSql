cmake_minimum_required(VERSION 3.16)
project(AsyncSql VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Sql)
find_package(Qt6 OPTIONAL_COMPONENTS Gui Widgets Qml Quick)

qt_standard_project_setup()

# ---- AsyncSql library ----

add_library(asyncsql STATIC
    asyncsql/asyncsqltablemodel.cpp
    asyncsql/asyncsqllistmodel.cpp
    asyncsql/asyncmodelregister.cpp
    asyncsql/databaseconnection.cpp
    asyncsql/databaseexception.cpp
    asyncsql/queryrequest.cpp
    asyncsql/queryresult.cpp
    asyncsql/querythread.cpp
    asyncsql/queryworker.cpp
    asyncsql/asyncsqltablemodel.h
    asyncsql/asyncsqllistmodel.h
    asyncsql/asyncmodelregister.h
    asyncsql/databaseconnection.h
    asyncsql/databaseexception.h
    asyncsql/queryrequest.h
    asyncsql/queryresult.h
    asyncsql/querythread.h
    asyncsql/queryworker.h
)

target_link_libraries(asyncsql PUBLIC Qt6::Core Qt6::Sql)

target_include_directories(asyncsql PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/asyncsql>
)

# The library uses standard Qt keywords (signals/slots/emit), bare string
# literals, and C++ exceptions — all incompatible with KDE's strict compiler
# flags.  Undefine/restore them for this target's own translation units only.
target_compile_options(asyncsql PRIVATE
    -UQT_NO_KEYWORDS
    -UQT_NO_CAST_FROM_ASCII
    -UQT_NO_CAST_FROM_BYTEARRAY
    -UQT_NO_CAST_TO_ASCII
    -UQT_USE_QSTRINGBUILDER
    -UQT_STRICT_ITERATORS
    -fexceptions
)

# ---- Examples ----
# To build examples: cmake -DBUILD_EXAMPLES=ON ..

option(BUILD_EXAMPLES "Build example applications" OFF)

if(BUILD_EXAMPLES)

    # Widgets example
    if(Qt6Gui_FOUND AND Qt6Widgets_FOUND)
        add_executable(AsyncSqlViewer
            examples/databaseviewer/main.cpp
            examples/databaseviewer/mainwindow.cpp
            examples/databaseviewer/mainwindow.h
            examples/databaseviewer/optionsdialog.cpp
            examples/databaseviewer/optionsdialog.h
            examples/databaseviewer/mainwindow.ui
            examples/databaseviewer/optionsdialog.ui
        )

        target_link_libraries(AsyncSqlViewer PRIVATE
            asyncsql
            Qt6::Gui
            Qt6::Widgets
        )

        target_include_directories(AsyncSqlViewer PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/examples/databaseviewer
        )
    endif()

    # QML table viewer example
    if(Qt6Qml_FOUND AND Qt6Quick_FOUND)
        add_executable(AsyncSqlQml
            examples/qmlviewer/main.cpp
            examples/qmlviewer/qml.qrc
        )

        target_link_libraries(AsyncSqlQml PRIVATE
            asyncsql
            Qt6::Qml
            Qt6::Quick
        )
    endif()

    # QML list viewer example (AsyncSqlListModel)
    if(Qt6Qml_FOUND AND Qt6Quick_FOUND)
        add_executable(AsyncSqlListViewer
            examples/listviewer/main.cpp
            examples/listviewer/qml.qrc
        )

        target_link_libraries(AsyncSqlListViewer PRIVATE
            asyncsql
            Qt6::Qml
            Qt6::Quick
        )
    endif()

endif() # BUILD_EXAMPLES
