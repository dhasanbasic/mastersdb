

add_executable(db_create db_create.cpp)
target_link_libraries(db_create PRIVATE mastersdb)
target_include_directories(db_create PRIVATE ../mastersdb)

add_executable(db_read db_read.cpp)
target_link_libraries(db_read PRIVATE mastersdb)
target_include_directories(db_read PRIVATE ../mastersdb)

add_executable(btree btree.c)
target_link_libraries(btree PRIVATE mdb)
target_include_directories(btree PRIVATE ../mastersdb)
