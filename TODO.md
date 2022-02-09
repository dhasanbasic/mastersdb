# TODO list for MastersDB project

## Last development (2010)
   * design and implement an object oriented wrapper around the database API
  
## Pending
   * extend the dummy implementations of mbdBtree{Read,Write}Node functions
   * design and implement the B+-tree structure (secondary indexes)
   * implement `SELECT` (multi-table, with `WHERE`) - partially implemented

## Finished
   * implement `SELECT` (single-table, with `WHERE`)
   * implement a B-tree with _order_ being the minimum children count instead of minimum record count
   * implement support for following data types:
     ```
     NAME     NAME LENGTH  HEADER  SIZE (B)  COMPARISON FUNCTION
     INT-8    5            0       1         Standard C -> memcmp
     INT-16   6            0       2         Standard C -> memcmp
     INT-32   6            0       4         Standard C -> memcmp
     FLOAT    5            0       4         assembly   -> CompareFloat
     STRING   6            4       N*1 + 4   Standard C -> strncmp
     ```
  * design and implement the missing type comparison functions:
    - `mdbCompareFloat`

  * design a file format for storing many B-trees
    ```
    MastersDB format magic number & version              4 bytes
    MastersDB format header
        16 entry free blocks table (8 bytes each)       (128 bytes)
    --------------------------------------------------------------
        TOTAL                                            132 bytes
    ```
    - B-tree for `.TABLES` table
    - B-tree for `.COLUMNS` table
    - B-tree for `.INDEXES` table
    - B-tree's for user-defined tables
    - ...

  * design and implement the three system tables

    - .Tables
      ```
      -----------------------------------------------------------------
      COLUMN   | TYPE   | LENGTH | SIZE | DESCRIPTION
      -----------------------------------------------------------------
      (#)Name  | STRING | 55     | 59   | Name of the table
      Fields   | INT-8  | N/A    | 1    | Number of columns
      B-tree   | INT-32 | N/A    | 4    | Pointer to B-tree descriptor
      -----------------------------------------------------------------
      Record size: 64 bytes
      ```

    - .Columns
      ```
      -----------------------------------------------------------------
      COLUMN         | TYPE   | LENGTH | SIZE | DESCRIPTION
      -----------------------------------------------------------------
      (#)Identifier  | STRING | 60     | 64   | Name of the column
      Name           | STRING | 54     | 58   | Name of the column
      Type           | INT-8  | N/A    | 1    | Column data type
      Indexed        | INT-8  | N/A    | 1    | Column is indexed
      Length         | INT-32 | N/A    | 4    | Maximum data length
      -----------------------------------------------------------------
      Record size: 128 bytes
      ```

    - .Indexes
      ```
      -----------------------------------------------------------------
      COLUMN         | TYPE   | LENGTH | SIZE | DESCRIPTION
      -----------------------------------------------------------------
      (#)Identifier  | STRING | 60     | 64   | Name of the column
      B+ tree        | INT-32 | N/A    | 4    | Pointer to B+ tree
      -----------------------------------------------------------------
      - Record size: 68 bytes
      ```

  * implement `mdbCreateDatabase* for creating an empty MastersDB database  

  * implement `mdbOpenDatabase` for loading database meta data into memory

  * implement `mdbCloseDatabase` for freeing used resources

  * refactoring of whole database

  * ensure that the mdbBtree* functions are aware of the data type of the
    primary key
  
  * implement table specific functions (`mdbCreateTable`, `mdbLoadTable`)
  
  * design a virtual machine for the SQL Parser/Engine
  
  * design an SQL Parser/Engine
  
  * implement `CREATE TABLE` statement
  
  * implement `INSERT INTO` statement

  * implement `DESC/DESCRIBE` statement
  
  * implement `SELECT` (single-table, without `WHERE`)

## Optimizations

   * General
     - Assume preallocated `BtreeNode` structures in `ReadNode`/`WriteNode` implementations.
            
   * `btree.c`:
     - Use binary search during insert/delete, instead of sequential.
     - Write iterative versions of insert/delete.

## Ideas
   * `btree.h`, `btree.c`
    - Instead of using `unsigned long` as child pointer to a node, use `void*`, store the child pointer size in the Btree structure and let `ReadNode`/`WriteNode` take care of what to do with that pointer. With such an implementation, in-memory and in-file B-trees can be easier implemented.
