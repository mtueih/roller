cmake_minimum_required(VERSION 4.4)
project(roller C)

set(CMAKE_C_STANDARD 11)

if(WIN32)
	add_executable(roller WIN32 main.c)
else()
	message(FATAL_ERROR "roller is a Windows GUI application and requires Win32 APIs")
endif()
