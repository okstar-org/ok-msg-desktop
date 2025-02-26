
set(gtest_force_shared_crt OFF CACHE BOOL "" FORCE)

# Disable pthread on google test
if(WIN32)
    # 禁用 pthread on Windows
    set(gtest_disable_pthread ON)
    set(MINGW ON)
    message(STATUS "Disable gtest pthread: ${gtest_disable_pthread}")
endif()


# 设置googletest的版本（可选）
set(GOOGLETEST_VERSION "release-1.12.0") # 替换为你想要的版本
FetchContent_Declare(googletest
                        GIT_REPOSITORY git@github.com:google/googletest.git
                        GIT_TAG ${GOOGLETEST_VERSION} # 如果设置了版本
)
FetchContent_MakeAvailable(googletest)

# 启用测试
enable_testing()

