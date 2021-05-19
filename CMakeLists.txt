# CMakeList.txt: udp_test 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.0)

project ("udp_test")

set(EXE_NAME "udp-test")

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	set(LINK_LIB Ws2_32 shell32 advapi32)
	set(UDP_TEST_SRC 
		getopt_long.c
		getopt.c
		udp_test.c)
else()
	set(LINK_LIB "")
	set(UDP_TEST_SRC 
		udp_test.c)
endif()

# 将源代码添加到此项目的可执行文件。
add_executable(${EXE_NAME} ${UDP_TEST_SRC})
target_link_libraries(${EXE_NAME} ${LINK_LIB})

# TODO: 如有需要，请添加测试并安装目标。
