add_library(${PROJECT_NAME} SHARED
    "mtbulk.cpp"
    "mtbulk.h"
    "${PROJECT_NAME}.cpp"
    "${PROJECT_NAME}.h"
    )

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        COMPILE_OPTIONS "-Wpedantic;-Wall;-Wextra"
        )
endif()

target_link_libraries(${PROJECT_NAME}
    ${CONAN_LIBS}
    )

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)
install(FILES "${PROJECT_NAME}.h" DESTINATION include)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT dinsul2.0@gmail.com)

include (CPack)
