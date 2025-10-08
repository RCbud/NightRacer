cmake_minimum_required(VERSION 3.22)
project(NightRacer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Пакети з vcpkg ----
find_package(glm CONFIG REQUIRED)
find_package(SFML 2.5 COMPONENTS system window graphics REQUIRED)
# ---- Ваш виконуваний файл ----
add_executable(${PROJECT_NAME} main.cpp)

# ---- Лінкування ----
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm sfml-system sfml-window sfml-graphics)


# ---- (Опційно) Скопіювати DLL SFML поруч з .exe при збиранні ----
if (WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
            COMMAND_EXPAND_LISTS
    )
endif()
