
set(OBJECT_SOURCES
  mdbVirtualMachine.h
  mdbVirtualMachine.cpp
  mdbVirtualTable.h
  mdbVirtualTable.cpp
  mdbQueryResults.h
  mdbQueryResults.cpp
)

add_library(mvm OBJECT ${OBJECT_SOURCES})

include_directories(../)

target_link_libraries(mvm PRIVATE $<TARGET_OBJECTS:mdb>)
