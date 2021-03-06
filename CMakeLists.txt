cmake_minimum_required (VERSION 2.6)

project (PersonaBoy)

SET(CMAKE_TOOLCHAIN_FILE "c++0x.toolchain.cmake") 
##### OPENGL
find_package(OpenGL REQUIRED)
INCLUDE_DIRECTORIES(${OpenGL_INCLUDE_DIRS})
set(LIBS ${LIBS} ${OPENGL_LIBRARIES})

##### BOOST
find_package(Boost REQUIRED COMPONENTS thread)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
set(LIBS ${LIBS} ${Boost_LIBRARIES})
#####

##### OPENCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
set(LIBS ${LIBS} ${OpenCV_LIBS})
#####

##### FREENECT
set(LIBS ${LIBS} freenect)
#####

##### QT stuff
find_package(Qt4 REQUIRED)
SET(QT_USE_QTOPENGL TRUE)
SET(QT_USE_QTNETWORK TRUE)

SET(SRC_DIR src)

FILE(GLOB project_SOURCES ${SRC_DIR}/*.c*)
FILE(GLOB project_HEADERS ${SRC_DIR}/*.h*)
FILE(GLOB project_FORMS forms/*.ui)
FILE(GLOB project_RESOURCES res/*.qrc)

INCLUDE_DIRECTORIES(${SRC_DIR})

SET(HEADERS_HAVING_Q_OBJECT)

foreach(header ${project_HEADERS})
    file(STRINGS "${header}" lines REGEX "Q_OBJECT")
    if(lines)
        list(APPEND HEADERS_HAVING_Q_OBJECT "${header}")
    endif()
endforeach()

QT4_WRAP_CPP(project_HEADERS_MOC ${HEADERS_HAVING_Q_OBJECT})
QT4_WRAP_UI(project_FORMS_HEADERS ${project_FORMS})
QT4_ADD_RESOURCES(project_RESOURCES_RCC ${project_RESOURCES})

INCLUDE(${QT_USE_FILE})

set(LIBS ${LIBS} ${QT_LIBRARIES}) 
######

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

### define all input sources, headers, moc, etc.
SET(ALL_STUFF 
    ${project_SOURCES}
    ${project_HEADERS}
    ${project_HEADERS_MOC} 
    ${project_FORMS_HEADERS} 
    ${project_RESOURCES_RCC})

# Source Groups affect grouping in IDEs like MSVC or XCode
SOURCE_GROUP("Headers" FILES ${project_HEADERS})
SOURCE_GROUP("Sources" FILES ${project_SOURCES})
SOURCE_GROUP("UI Forms" FILES ${project_FORMS})
SOURCE_GROUP("Resources" FILES ${project_RESOURCES})
SOURCE_GROUP("QT generated" FILES ${project_HEADERS_MOC} ${project_FORMS_HEADERS}
                ${project_RESOURCES_RCC})

SET(PROGNAME cvQT)

IF( APPLE )
    SET( MACOSX_BUNDLE_ICON_FILE persona.icns )

file(GLOB_RECURSE resFiles
  ${CMAKE_SOURCE_DIR}/persona.icns
)

SET_SOURCE_FILES_PROPERTIES(
  ${resFiles}
  PROPERTIES
  MACOSX_PACKAGE_LOCATION Resources
)

ADD_EXECUTABLE(${PROGNAME} MACOSX_BUNDLE ${ALL_STUFF} ${resFiles})

ELSE( APPLE )
ADD_EXECUTABLE(${PROGNAME} ${ALL_STUFF})
ENDIF( APPLE )

TARGET_LINK_LIBRARIES(${PROGNAME} ${LIBS})

INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

### INSTALL
#INSTALL (TARGETS ${PROGNAME} DESTINATION bin)
