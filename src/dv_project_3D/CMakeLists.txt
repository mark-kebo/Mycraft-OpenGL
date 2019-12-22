project(dv_project_3D)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILER_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${COMPILER_RELEASE_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${COMPILER_DEBUG_FLAGS}")

file(GLOB_RECURSE ${PROJECT_NAME}_headers *.h *.hpp)
file(GLOB_RECURSE ${PROJECT_NAME}_sources *.cpp)

if(NOT APPLE)
	if(WIN32)
		find_package(GLFW REQUIRED)
		include_directories(${GLFW_INCLUDE_DIRS})
		set(${PROJECT_NAME}_LIBS ${${PROJECT_NAME}_LIBS} ${GLFW_LIBRARIES})
	endif()

        find_package(glbinding REQUIRED)
        include_directories(${glbinding_INCLUDE_DIRS})
        set(${PROJECT_NAME}_LIBS ${${PROJECT_NAME}_LIBS} ${glbinding_LIBRARIES})
endif()

add_executable (${PROJECT_NAME} ${${PROJECT_NAME}_headers} ${${PROJECT_NAME}_sources})

target_link_libraries(${PROJECT_NAME}
                      OGLAppFramework
                      ${${PROJECT_NAME}_LIBS}
                      ${${PROJECT_NAME}_LIBS_DEBUG}
                      ${LINKER_FLAGS})

install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION dll
        ARCHIVE DESTINATION lib)
