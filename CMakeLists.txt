project (Myserver)

add_subdirectory(client)

aux_source_directory(. DIR_SRCS)

add_executable(server ${DIR_SRCS})
