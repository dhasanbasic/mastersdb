
set(OBJECT_SOURCES
  mdb.h
  mdbtypes.h
  mdbbtree_util.h
  mdbbtree.c
  mdbdatabase.c
  mdbtable.c
)

add_library(mdb OBJECT ${OBJECT_SOURCES})

add_subdirectory(mql)
add_subdirectory(mvm)
