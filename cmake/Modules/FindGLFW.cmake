set(GLFW_FOUND "NO")
set(GLFW_INCLUDE_DIRS "NOTFOUND")
set(GLFW_LIBRARIES "NOTFOUND")

if(WIN32 AND NOT UNIX)
	set(GLFW_LIBRARIES_OPT optimized "${MAIN_PROJECT_DIR}/libs/glfw/lib/Win/x64/GLFW3.lib")
	set(GLFW_LIBRARIES_DEBUG debug "${MAIN_PROJECT_DIR}/libs/glfw/lib/Win/x64/GLFW3.lib")

	set(GLFW_INCLUDE_DIRS "${MAIN_PROJECT_DIR}/libs/glfw/include")

	set(GLFW_FOUND "YES")
else()
    message("There is no version for Linux/Apple!")
endif()

set(GLFW_LIBRARIES ${GLFW_LIBRARIES_OPT} ${GLFW_LIBRARIES_DEBUG})

message("GLFW found: ${GLFW_FOUND}")
message("GLFW include dir: ${GLFW_INCLUDE_DIRS}")
message("GLFW libraries: ${GLFW_LIBRARIES}")
