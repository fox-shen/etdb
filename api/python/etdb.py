#encoding=utf-8
import os, sys
from sys import stdin, stdout
import socket

class ETDB(object):
  def __init__(this,host,port):
    this.recv_buf = ''
    this.closed   = False
    this.sock     = socket.socket(socket.AF_INET, socket.SOCK_STREAM);
    this.sock.connect(tuple([host, port]))
    this.sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)

  def close(this):
    if not this.closed:
      this.sock.close()
      this.closed = True

  def closed(this):
    return this.closed

  def request(this, cmd, params = None):
    if params == None:
      params = []
    params = [cmd] + params
    this.send(params)
    resp   = this.recv()
    if resp == None:
      return 'error'
    if len(resp) == 0:
      return 'disconnectd'
    #### parse command response.
    return 'ok'

  def send(this, data):
    ds    = []
    for item in data:
      ds.append(str(len(item)))
      ds.append(item)
    ds.append("0")
    n1    = '\n'
    s     = n1.join(ds) + '\n\n'
    print s
    ####
    try:
      while True:
        ret = this.sock.send(s)
        if ret == 0:
          return -1
        s = s[ret: ]
        if len(s) == 0:
          break
    except socket.error, e:
      return -1
    return ret

  def recv(this):
    while True:
      ret = this.parse()
      if ret == None:
        if this.read() == 0:
          return []
      else
        return ret

  def read(this):
    try:
      data = this.sock.recv(1024*8)
    except Exception, e:
      data = ''
    if data == '':
      this.close()
      return 0
    this.recv_buf += data
    return len(data)

  def parse(this):
    return 0

if __name__ == "__main__":
  etdb = ETDB("127.0.0.1", 19000)
  etdb.request("set", ["abc", "egf"])
  while True:
    print "a"  
