-----------------------------------
  TODO list for MastersDB project
-----------------------------------
                      Copyright (C) 2010, Dinko Hasanbasic (dinkoh@bih.net.ba)

Active development
------------------
  * implement mdbCloseDatabase for freeing used resources
  * extend the dummy implementations of mbdBtree{Read,Write}Node functions
  
Pending
-------
  * ensure that the mdbBtree* functions are aware of the data type of the
    primary key
  * design and implement an SQL Engine/Parser
  * design and implement an object oriented wrapper around the database API
  * design and implement the B+-tree structure (secondary indexes)

Finished
--------
  * implement a B-tree with "order" being the minimum children count
    instead of minimum record count
    
  * implement support for following data types:
    ** NAME     NAME LENGTH  HEADER  SIZE (B)  COMPARISON FUNCTION
       INT-8    5            0       1         Standard C -> memcmp
       INT-16   6            0       2         Standard C -> memcmp
       INT-32   6            0       4         Standard C -> memcmp
       FLOAT    5            0       4         assembly   -> CompareFloat
       CHAR-8   6            4       N*1 + 4   Standard C -> strncmp
       
    ** design and implement the missing type comparison functions:
      - mdbCompareFloat

  * design a file format for storing many B-trees
    # MastersDB format magic number & version              4 bytes
    # MastersDB format header
      - 3 entry B-tree descriptor table (system tables) ( 12 bytes)
      - 20 entry B-tree descriptor table                ( 80 bytes)
      - 20 entry free blocks table (8 bytes each)       (160 bytes)
      --------------------------------------------------------------
      - TOTAL                                            256 bytes
    # B-tree for _TABLES table
    # B-tree for _FIELDS table
    # B-tree for _INDEXES table
    # B-tree's for user-defined tables
    # ...

  * design and implement the three system tables

    # _TABLES
      -----------------------------------------------------------------
      FIELD    | TYPE   | LENGTH | SIZE | DESCRIPTION
      -----------------------------------------------------------------
      (#)Name  | STRING | 55     | 59   | Name of the table
      Fields   | INT-8  | N/A    | 1    | Number of fields
      B-tree   | INT-32 | N/A    | 4    | Pointer to B-tree descriptor
      -----------------------------------------------------------------
      - Record size: 64 bytes

    # _FIELDS
      -----------------------------------------------------------------
      FIELD          | TYPE   | LENGTH | SIZE | DESCRIPTION
      -----------------------------------------------------------------
      (#)Identifier  | STRING | 60     | 64   | Name of the field
      Type           | STRING | 8      | 12   | Name of the data type
      Name           | STRING | 43     | 47   | Name of the field
      Indexed        | INT-8  | N/A    | 1    | Field is indexed
      Length         | INT-32 | N/A    | 4    | Maximum data length
      -----------------------------------------------------------------
      - Record size: 128 bytes

    # _INDEXES
      -----------------------------------------------------------------
      FIELD          | TYPE   | LENGTH | SIZE | DESCRIPTION
      -----------------------------------------------------------------
      (#)Identifier  | STRING | 60     | 64   | Name of the field
      B+ tree        | INT-32 | N/A    | 4    | Pointer to B+ tree
      -----------------------------------------------------------------
      - Record size: 68 bytes

  * implement mdbCreateDatabase for creating an empty MastersDB database
  
  * implement mdbOpenDatabase for loading database meta data into memory

Optimizations
-------------
  * General
    - Assume preallocated BtreeNode structures in ReadNode/WriteNode
      implementations.
            
  * btree.c:
    - Use binary search during insert/delete, instead of sequential.
    - Write iterative versions of insert/delete.

Ideas
-----
  * btree.h, btree.c
    - Instead of using unsigned long as child pointer to a node, use void*,
      store the child pointer size in the Btree structure and let ReadNode/
      WriteNode take care of what to do with that pointer. With such
      implementation, in-memory and in-file B-trees can be easier implemented.
