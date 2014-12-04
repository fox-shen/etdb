### Concurrency benchmark
```
========== set ==========
redis -> qps: 133265
etdb  -> qps: 100000
========== get ==========
redis -> qps: 135541
etdb  -> qps: 135135
========== del ==========
redis -> qps: 
etdb  -> qps: 121951
```

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
  % spset id1 30 120
  % spset id2 30.1 120.1
  $ sprect 29.5 30.2 119 121
  id1
  id2
```
  
