Building etdb
============
etdb can be compiled and used on Linux. building is as simple as:
```building
  % ./configure
  % make
```
Running etdb
============
running etdb with config defined by user
```running
  % ./objs/etdb_server conf/etdb.conf
  % 
```
Playing with etdb
=============
you can use etdb_cli to play with etdb
```running
  % ./objs/etdb_cli 127.0.0.1 19000
  % set a b
  % get a
  b
  % siset id1 30 120
  % siset id2 30.1 120.1
  $ sirect 29.5 30.2 119 121
  id1
  id2
```
  
