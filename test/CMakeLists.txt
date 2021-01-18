cmake_minimum_required(VERSION 3.8)

set(testname "test_threadpool")

set (SRC "TestThreadPool.cpp")

add_executable("${testname}" "${SRC}")

target_link_libraries("${testname}" PRIVATE
		"${libname}"
		gtest_main
		)

add_test(
	NAME ${testname}
	COMMAND ${testname})


