project(SpinLock)

add_executable(${PROJECT_NAME} main.cpp)

find_package(Threads)
if(${Threads_FOUND})
    if(${CMAKE_USE_WIN32_THREADS_INIT})
        message("[moon] found win32 thread")
    elseif(${CMAKE_USE_PTHREADS_INIT})
        message("[moon] found pthread")
    endif()
endif()
target_link_libraries (${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})