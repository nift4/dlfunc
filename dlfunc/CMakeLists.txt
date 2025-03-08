# Sets the minimum version of CMake required to build your native library.
# This ensures that a certain set of CMake features is available to
# your build.

cmake_minimum_required(VERSION 3.4.1)
project(dlfunc)

# Specifies a library name, specifies whether the library is STATIC or
# SHARED, and provides relative paths to the source code. You can
# define multiple libraries by adding multiple add_library() commands,
# and CMake builds them for you. When you build your app, Gradle
# automatically packages shared libraries with your APK.

find_library( # Defines the name of the path variable that stores the
        # location of the NDK library.
        log-lib

        # Specifies the name of the NDK library that
        # CMake needs to locate.
        log )


add_library( # Specifies the name of the library.
             dlfunc

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/jni/dlfunc.c )

target_link_libraries( # Specifies the target library.
        dlfunc

        # Links the log library to the target library.
        ${log-lib} )

if(ANDROID AND ANDROID_ABI MATCHES "arm64-v8a")
	target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -fPIC -fomit-frame-pointer)
else()
	target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -fPIC -fomit-frame-pointer -Oz)
endif()

add_custom_command(TARGET dlfunc
		POST_BUILD
		COMMAND ${CMAKE_OBJCOPY} --remove-section .comment --remove-section .note.gnu.build-id $<TARGET_FILE:dlfunc>
		VERBATIM
)