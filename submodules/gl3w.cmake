# 2024

# find python interpreter
find_package(PythonInterp REQUIRED)

# generate files
execute_process(
    COMMAND ${CMAKE_COMMAND} -E make_directory gl3w
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
execute_process(
    COMMAND "${PYTHON_EXECUTABLE}" ${CMAKE_CURRENT_SOURCE_DIR}/gl3w/gl3w_gen.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/gl3w
)