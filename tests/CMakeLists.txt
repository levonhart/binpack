add_executable(readfile)
target_sources( readfile
				PUBLIC "readfile.c")
target_link_libraries( readfile PUBLIC binpack)

add_executable(solution)
target_sources( solution
				PUBLIC "solution.c")
target_link_libraries( solution PUBLIC binpack)

add_executable(search)
target_sources( search
				PUBLIC "search.c")
target_link_libraries( search PUBLIC binpack)

set_target_properties( readfile solution search
						PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED ON)

if(NOT MSVC_DLL)
	set_target_properties( readfile solution search
							PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
endif()
