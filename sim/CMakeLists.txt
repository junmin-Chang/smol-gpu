set(EXEC_NAME "simulator")

add_library(Sim INTERFACE sim.hpp)
target_include_directories(Sim INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${EXEC_NAME} main.cpp)

set(MAIN_FLAGS ${COMPILE_FLAGS})

if(SANITIZER_AVAILABLE_AND_SET)
  set(MAIN_FLAGS ${MAIN_FLAGS} ${SANITIZER_FLAGS})
  target_link_libraries(${EXEC_NAME} ${SANITIZER_FLAGS})
endif()

target_link_libraries(${EXEC_NAME} Sim)

target_compile_options(${EXEC_NAME} PRIVATE ${MAIN_FLAGS})

target_link_libraries(${EXEC_NAME} GPU)
