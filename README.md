### Concurrency benchmark
```
========== set ==========
qps: 44251, time: 0.226 s
========== get ==========
qps: 55541, time: 0.180 s
========== del ==========
qps: 46080, time: 0.217 s
========== hset ==========
qps: 42338, time: 0.236 s
========== hget ==========
qps: 55601, time: 0.180 s
========== hdel ==========
qps: 46529, time: 0.215 s
========== zset ==========
qps: 37381, time: 0.268 s
========== zget ==========
qps: 41455, time: 0.241 s
========== zdel ==========
qps: 38792, time: 0.258 s
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
  % siset id1 30 120
  % siset id2 30.1 120.1
  $ sirect 29.5 30.2 119 121
  id1
  id2
```
  
