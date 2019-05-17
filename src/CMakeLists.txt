cmake_minimum_required(VERSION 3.2)
project(FrustumCulling)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_library(FREETYPE_LIB freetype HINTS ${PROJECT_BINARY_DIR})
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

add_definitions(${GLM_DEFINITIONS})

include_directories(
	${PROJECT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/freetype
	${PROJECT_SOURCE_DIR}/freetype/config
    ${OPENGL_INCLUDE_DIRS} 
    ${GLUT_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
	)
    
file(GLOB_RECURSE ${CMAKE_PROJECT_NAME}_src
	"*.h"
	"*.hpp"
	"*.cpp"
	)

add_executable(${CMAKE_PROJECT_NAME} ${${CMAKE_PROJECT_NAME}_src})

target_link_libraries(${CMAKE_PROJECT_NAME}
	${FREETYPE_LIB}
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARY}
    ${GLEW_LIBRARIES}
	)

################## DOXYGEN ################## 

# first we can indicate the documentation build as an option and set it to ON by default
option(BUILD_DOC "Build documentation" ON)

# check if Doxygen is installed
find_package(Doxygen)
if (DOXYGEN_FOUND)
	# set input and output files
	set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/../doc/Doxyfile.in)
	set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/../doc/Doxyfile)

	# request to configure the file
	configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
	message("Doxygen build started")

	add_custom_target(doc ALL
		COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../doc)
else (DOXYGEN_FOUND)
	message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)

################## PACKAGING ################## 

add_custom_target(pack COMMAND
	${CMAKE_COMMAND} -E tar "cfv" "project.zip" --format=zip
	"${CMAKE_CURRENT_SOURCE_DIR}"
	)
