cmake_minimum_required(VERSION 3.23)
project(ex3b_noor_khamaisy C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(ex3b_noor_khamaisy
        linked_list.c
        linked_list.h
        markov_chain.h
        snakes_and_ladders.c)
