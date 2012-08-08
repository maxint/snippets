set(PROJ_NAME Pipe)

include_directories(
    ${OGRE_INCLUDE_DIRS}
    #${OIS_INCLUDE_DIRS}
    #$ENV{OGRE_SOURCE}/Dependencies/include
    #$ENV{OGRE_SOURCE}/Samples/Common/include # source version
    include
    )

## Files
file(GLOB SRCS *.cpp)
file(GLOB HDRS *.h)
#set(HDRS
    #Application.h
    #GameLogic.h
    #MainWindow.h
    #OgreWidget.h)

#set(SRCS
    #main.cpp
    #Application.cpp
    #GameLogic.cpp
    #MainWindow.cpp
    #OgreWidget.cpp)

list(REMOVE_ITEM 
    SRCS
    FrameData.cpp
    )

list(REMOVE_ITEM 
    HDRS
    FrameData.h
    )


set(MOC_HDRS
    Application.h
    OgreWidget.h
    EventHandlingOgreWidget.h
    MainWindow.h
    )

#list(REMOVE_ITEM HDRS MOC_HDRS)

file(GLOB QRC_FILE *.qrc)

## QT
qt4_wrap_cpp(MOC_SRCS ${MOC_HDRS})

qt4_add_resources(QRC_SRC ${QRC_FILE})
source_group("Resource Files" FILES
    ${QRC_FILE}
    )

source_group("Generated Files" FILES
    ${QRC_SRC}
    ${MOC_SRCS}
    )

## Output Qt message
message(STATUS "--------------------------------------------------------------------")
message(STATUS "-- Files generated for Qt project ")
message(STATUS "+ qrc file : " ${QRC_FILE})
message(STATUS "+ qrc cxx : " ${QRC_SRC})
message(STATUS "+ moc header: " ${MOC_HDRS})
message(STATUS "+ moc src: " ${MOC_SRCS})
message(STATUS "--------------------------------------------------------------------")


## Create exe file

add_executable(${PROJ_NAME}
    #WIN32 # SUBSYSTEM:WINDOWS
    #EXCLUDE_FROM_ALL
    ${QRC_SRC}
    ${MOC_SRCS}
    ${HDRS}
    ${SRCS}
    )

target_link_libraries(${PROJ_NAME}
    #${OIS_LIBRARIES}
    ${OGRE_LIBRARIES}
    ${QT_QTMAIN_LIBRARY}
    ${QT_LIBRARIES}
    )

if (MSVC)
    set(CT_WORK_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}\\$(ConfigurationName)")
    set(CT_ENV_DIR_ADDED "$(OGRE_BUILD)\\bin\\$(ConfigurationName);$(QTDIR)\\bin")
    create_vcproj_userfile( ${PROJ_NAME} )
endif ()
