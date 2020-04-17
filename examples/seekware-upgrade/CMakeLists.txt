#Version-------------------------------------------------------------------------------------------------------------------#

#Sources-------------------------------------------------------------------------------------------------------------------#

set(SOURCES 
	src/seekware-upgrade.c
)

#Project-------------------------------------------------------------------------------------------------------------------#

project(seekware-upgrade DESCRIPTION "Seekware SDK Library")

#Targets-------------------------------------------------------------------------------------------------------------------#

add_executable(${PROJECT_NAME} ${SOURCES})

#Dependencies---------------------------------------------------------------------------------------------------------------#

#Extra Compile Options------------------------------------------------------------------------------------------------------#

#Include--------------------------------------------------------------------------------------------------------------------#

include_directories(
	${CMAKE_SOURCE_DIR}/include
)

#Link------------------------------------------------------------------------------------------------------------------------#

#seekware
target_link_libraries(${PROJECT_NAME} seekware)

#Runtime Dependencies--------------------------------------------------------------------------------------------------------#

if(MSVC)

	get_target_property(SEEKWARE_IMPORT seekware IMPORTED)
	if(SEEKWARE_IMPORT)
		add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<TARGET_FILE_DIR:seekware>/seekware.dll
			$<TARGET_FILE_DIR:${PROJECT_NAME}>/seekware.dll
		)
	endif()
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		$<TARGET_FILE_DIR:usb-1.0>/libusb-1.0.dll
		$<TARGET_FILE_DIR:${PROJECT_NAME}>/libusb-1.0.dll
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
