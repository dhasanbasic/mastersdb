-----------------------------------
  TODO list for MastersDB project
-----------------------------------
                      Copyright (C) 2010, Dinko Hasanbasic (dinkoh@bih.net.ba)

General
-------
  * ISO-8859 character sets support
    - Create two binary files containing the ISO-8859 to UTF-8 and UTF-8 to
      ISO-8859 data. Eventually, store the UTF-8 to ISO-8859 mapping in an
      in-memory B-tree, so the corresponding ISO-8859 byte-code can be quickly
      accessed by its UTF-8 byte sequence (used as the key).
  
  * UTF-8 related
    - Implement a GetUTF8StringLength function.
      (http://www.cprogramming.com/tutorial/unicode.html) 

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
