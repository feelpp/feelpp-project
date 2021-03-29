set(APPLICATION_DOC_DIR "${CMAKE_INSTALL_DOCDIR}")

set(ANTORA_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/public")

find_program (npm NAMES npm)

if ( NPM_FOUND )

add_custom_command(OUTPUT "${ANTORA_OUTPUT_DIR}" ALL
    COMMENT "Building documentation... "
    COMMAND "${npm}" run antora 
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_custom_target(doc ALL DEPENDS "${ANTORA_OUTPUT_DIR}")

install(DIRECTORY "${ANTORA_OUTPUT_DIR}/" DESTINATION ${APPLICATION_DOC_DIR} OPTIONAL)
else()
    message(STATUS "npm is not installed, can use antora ")
endif()