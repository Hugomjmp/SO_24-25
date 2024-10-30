cmake_minimum_required(VERSION 3.29)
project(SO_24_25 C)

set(CMAKE_C_STANDARD 11)

include_directories(feed/header)
include_directories(manager/header)

add_executable(SO_24_25
    feed/header/feed.h
    feed/feed.c
    manager/header/manager.h
    manager/manager.c)
