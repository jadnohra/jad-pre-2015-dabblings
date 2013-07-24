if(NOT EXISTS "Z:/Personal/Lab/Physics/lamby/flann/build/install_manifest.txt")
    message(FATAL_ERROR "Cannot find install manifest: \"Z:/Personal/Lab/Physics/lamby/flann/build/install_manifest.txt\"")
endif(NOT EXISTS "Z:/Personal/Lab/Physics/lamby/flann/build/install_manifest.txt")

file(READ "Z:/Personal/Lab/Physics/lamby/flann/build/install_manifest.txt" files)
string(REGEX REPLACE "\n" ";" files "${files}")
foreach(file ${files})
    message(STATUS "Uninstalling \"$ENV{DESTDIR}${file}\"")
    if(EXISTS "$ENV{DESTDIR}${file}")
        exec_program("D:/jad2/Progs/cmake-2.8.11.2-win32-x86/bin/cmake.exe" ARGS "-E remove \"$ENV{DESTDIR}${file}\""
            OUTPUT_VARIABLE rm_out RETURN_VALUE rm_retval)
        if(NOT "${rm_retval}" STREQUAL 0)
            message(FATAL_ERROR "Problem when removing \"$ENV{DESTDIR}${file}\"")
        endif(NOT "${rm_retval}" STREQUAL 0)
    else(EXISTS "$ENV{DESTDIR}${file}")
        message(STATUS "File \"$ENV{DESTDIR}${file}\" does not exist.")
    endif(EXISTS "$ENV{DESTDIR}${file}")
endforeach(file)

