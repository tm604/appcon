add_library(
	appcon
	STATIC
		config.cpp
)
install(
	TARGETS appcon
	ARCHIVE DESTINATION lib
)
