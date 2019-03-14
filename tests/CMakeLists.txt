
link_directories("${CMAKE_BINARY_DIR}/lib")

link_libraries(${PROJECT_NAME})

add_executable("test_${PROJECT_NAME}"
    "test_${PROJECT_NAME}.cpp"
    )

