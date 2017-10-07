add_executable(testdebug testdebug.c)
target_link_libraries(testdebug log)

add_executable(testdebugcolor testdebugcolor.c)
target_link_libraries(testdebugcolor log)

add_executable(testinfo testinfo.c)
target_link_libraries(testinfo log)
