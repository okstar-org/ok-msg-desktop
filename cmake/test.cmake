
include(FetchContent)

# 设置googletest的版本（可选）
set(GOOGLETEST_VERSION "release-1.11.0") # 替换为你想要的版本
FetchContent_Declare(
		googletest
		GIT_REPOSITORY git@github.com:google/googletest.git
		GIT_TAG ${GOOGLETEST_VERSION} # 如果设置了版本
)

# 为了简化，我们不将gtest安装到系统路径中，而是直接在构建目录中构建和使用它
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
# 启用测试
enable_testing()
# Disable pthread on google test
if(WIN32)
	add_definitions(-DGTEST_HAS_PTHREAD=0)
endif()