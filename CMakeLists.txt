cmake_minimum_required(VERSION 3.16)

project(SkyMeetQmlClientQt515 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 5.15.2 REQUIRED COMPONENTS Core Quick Qml Network WebSockets)

qt5_add_resources(QML_RESOURCES qml.qrc)

add_executable(skymeet_qml_client_qt515
    src/main.cpp
    src/ApiClient.h
    src/ApiClient.cpp
    src/EventSocket.h
    src/EventSocket.cpp
    ${QML_RESOURCES}
)

target_link_libraries(skymeet_qml_client_qt515
    PRIVATE
        Qt5::Core
        Qt5::Quick
        Qt5::Qml
        Qt5::Network
        Qt5::WebSockets
)

if(WIN32)
    set_target_properties(skymeet_qml_client_qt515 PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
