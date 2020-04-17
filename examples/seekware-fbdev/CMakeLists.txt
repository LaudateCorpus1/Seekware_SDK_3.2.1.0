#Version-------------------------------------------------------------------------------------------------------------------#

#Sources-------------------------------------------------------------------------------------------------------------------#

set(SOURCES 
	src/seekware-fbdev.c
)

#Project--------------------------------------------------------------------------------------------------------------------#

project(seekware-fbdev DESCRIPTION "Seekware SDK Library")

#Targets---------------------------------------------------------------------------------------------------------------------#

add_executable(${PROJECT_NAME} ${SOURCES})

#Dependencies----------------------------------------------------------------------------------------------------------------#

#Extra Compile Options------------------------------------------------------------------------------------------------------#

#Include--------------------------------------------------------------------------------------------------------------------#

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

#Link------------------------------------------------------------------------------------------------------------------------#

#seekware
target_link_libraries(${PROJECT_NAME} seekware)

#Install---------------------------------------------------------------------------------------------------------------------#

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

if(CMAKE_CROSSCOMPILING)
	install(TARGETS ${PROJECT_NAME} DESTINATION install/bin)

	file(GLOB SRC ${CMAKE_CURRENT_SOURCE_DIR}}/src/*)
	install(FILES ${SRC} DESTINATION install/src/${PROJECT_NAME}/src)

	file(GLOB INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*)
	install(FILES ${INC} DESTINATION install/src/${PROJECT_NAME}/include)
	
	install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/Makefile DESTINATION install/src/${PROJECT_NAME})
endif()