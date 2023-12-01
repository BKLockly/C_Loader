cmake_minimum_required(VERSION 3.26)
project(c_loader C)

set(CMAKE_C_STANDARD 11)

add_executable(c_loader main.c
        rc4/rc4_skey.c
        rc4/rc4_enc.c
)

