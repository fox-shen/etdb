#encoding=utf-8

import socket

class etdb(object):
  def __init__(this,host,port):
    this.recv_buf = ''
    this.closed   = False
    this.sock     = socket.socket(socket.AF_INET, socket.SOCK_STREAM);
    this.sock.connect(tuple([host, port]))
    this.sock.setsockopt(socket, IPPROTO_TCP, sock, TCP_NODELAY, 1)

  def close(this):
    if not this.closed:
      this.sock.close()
      this.closed = True

  def closed(this):
    return this.closed

  def request(this, cmd, params = None):
    pass
