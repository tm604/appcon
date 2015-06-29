
add_executable(
	appcon_tests
	main.cpp
	cfgmaker.cpp
	config.cpp
	commandline.cpp
	environ.cpp
	file.cpp
)
target_link_libraries(
	appcon_tests
	appcon
	${Boost_LIBRARIES}
)
if(THREADS_HAVE_PTHREAD_ARG)
	target_compile_options(PUBLIC appcon_tests "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
	target_link_libraries(appcon_tests "${CMAKE_THREAD_LIBS_INIT}")
endif()

add_test (appcon_tests appcon_tests -r junit -o appcon_tests.xml)

