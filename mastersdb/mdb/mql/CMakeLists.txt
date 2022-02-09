
set(OBJECT_SOURCES
  Buffer.h
  Buffer.cpp
  Scanner.h
  Scanner.cpp
  Parser.h
  Parser.cpp
  MQLSelect.h
  MQLSelect.cpp
)

add_library(mql OBJECT ${OBJECT_SOURCES})

include_directories(../)

target_link_libraries(mql PRIVATE $<TARGET_OBJECTS:mvm>)
