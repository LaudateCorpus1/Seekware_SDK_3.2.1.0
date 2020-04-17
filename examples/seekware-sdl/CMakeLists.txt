#Version-------------------------------------------------------------------------------------------------------------------#

#Sources-------------------------------------------------------------------------------------------------------------------#

set(SOURCES 
	src/seekware-sdl.c
)

#Project-------------------------------------------------------------------------------------------------------------------#

project(seekware-sdl DESCRIPTION "Seekware SDL Viewer")

#Targets-------------------------------------------------------------------------------------------------------------------#

add_executable(${PROJECT_NAME} ${SOURCES})
if(MSVC)
	set_property(GLOBAL PROPERTY VS_STARTUP_PROJECT seekware-sdl)
endif()

#Extra Compile Options------------------------------------------------------------------------------------------------------#

#Include--------------------------------------------------------------------------------------------------------------------#

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

if (SDL2_FOUND)
  include_directories(${SDL2_EXEC_PREFIX}/include)
endif (SDL2_FOUND)

#Link------------------------------------------------------------------------------------------------------------------------#

#SDL2
target_link_libraries(${PROJECT_NAME} SDL2)

if(SDL2_FOUND)
	target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
endif()

#seekware
target_link_libraries(${PROJECT_NAME} seekware)

#Runtime Dependencies--------------------------------------------------------------------------------------------------------#
if(MSVC)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		$<TARGET_FILE_DIR:seekware>/seekware.dll
		$<TARGET_FILE_DIR:${PROJECT_NAME}>/seekware.dll
	)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		$<TARGET_FILE_DIR:usb-1.0>/libusb-1.0.dll
		$<TARGET_FILE_DIR:${PROJECT_NAME}>/libusb-1.0.dll
	)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		$<TARGET_FILE_DIR:SDL2>/SDL2.dll
		$<TARGET_FILE_DIR:${PROJECT_NAME}>/SDL2.dll
	)
endif()

#Install---------------------------------------------------------------------------------------------------------------------#

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

if(CMAKE_CROSSCOMPILING)
	install(TARGETS ${PROJECT_NAME} DESTINATION install/bin)

	file(GLOB SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*)
	install(FILES ${SRC} DESTINATION install/src/${PROJECT_NAME}/src)
	
	file(GLOB INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*)
	install(FILES ${INC} DESTINATION install/src/${PROJECT_NAME}/include)

	install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/Makefile DESTINATION install/src/${PROJECT_NAME})
endif()
