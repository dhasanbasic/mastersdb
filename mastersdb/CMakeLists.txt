
set(LIBRARY_SOURCES
  MastersDB.h
  MdbDatabase.cpp
  MdbResultSet.cpp
)

add_subdirectory(mdb)

add_library(mastersdb ${LIBRARY_SOURCES})

target_include_directories(mastersdb PRIVATE mdb)
target_link_libraries(mastersdb PRIVATE mdb mvm mql)
