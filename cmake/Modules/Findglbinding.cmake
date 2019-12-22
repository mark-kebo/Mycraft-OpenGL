set(glbinding_FOUND "NO")
set(glbinding_INCLUDE_DIRS "NOTFOUND")
set(glbinding_LIBRARIES "NOTFOUND")

if(WIN32 AND NOT UNIX)
    set(glbinding_LIBRARIES_OPT optimized "${MAIN_PROJECT_DIR}/libs/glbinding/lib/Win/x64/glbinding.lib" optimized "${MAIN_PROJECT_DIR}/libs/glbinding/lib/Win/x64/glbinding-aux.lib")
    set(glbinding_LIBRARIES_DEBUG debug "${MAIN_PROJECT_DIR}/libs/glbinding/lib/Win/x64/glbinding.lib" debug "${MAIN_PROJECT_DIR}/libs/glbinding/lib/Win/x64/glbinding-aux.lib")

    set(glbinding_INCLUDE_DIRS "${MAIN_PROJECT_DIR}/libs/glbinding/include" "${MAIN_PROJECT_DIR}/libs/glbinding/Win/include")

    set(glbinding_FOUND "YES")
else()
    if(NOT APPLE)
        set(glbinding_LIBRARIES_OPT optimized "${MAIN_PROJECT_DIR}/libs/glbinding/lib/Linux/linux64/libglbinding.a" optimized "${MAIN_PROJECT_DIR}/libs/glbinding/lib/Linux/linux64/libglbinding-aux.a")
        set(glbinding_LIBRARIES_DEBUG debug "${MAIN_PROJECT_DIR}/libs/glbinding/lib/Linux/linux64/libglbinding.a" debug "${MAIN_PROJECT_DIR}/libs/glbinding/lib/Linux/linux64/libglbinding-aux.a")

        set(glbinding_INCLUDE_DIRS "${MAIN_PROJECT_DIR}/libs/glbinding/include" "${MAIN_PROJECT_DIR}/libs/glbinding/Linux/include")

        set(glbinding_FOUND "YES")
    else()
        message("There is no version for Apple!")
    endif()
endif()

set(glbinding_LIBRARIES ${glbinding_LIBRARIES_OPT} ${glbinding_LIBRARIES_DEBUG})

message("glbinding found: ${glbinding_FOUND}")
message("glbinding include dir: ${glbinding_INCLUDE_DIRS}")
message("glbinding libraries: ${glbinding_LIBRARIES}")
