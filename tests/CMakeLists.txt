include_directories(../src)

# Test filters
qt6_add_executable(test_filters
	test_filters/test_filters.cpp
	../src/filter_model.cpp
	../src/session.cpp
	../src/session_model.cpp
)
target_link_libraries(test_filters PRIVATE Qt6::Core Qt6::Test)
add_test(NAME test_filters COMMAND test_filters)

# Test sessions
qt6_add_executable(test_sessions
	test_sessions/test_sessions.cpp
	../src/session.cpp
	../src/session_model.cpp
)
target_link_libraries(test_sessions PRIVATE Qt6::Core Qt6::Test)
add_test(NAME test_sessions COMMAND test_sessions)
