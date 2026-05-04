#!/bin/sh
cvscmd="cvs -z 6 -d :pserver:anonymous@cvs.m17n.org:/cvs/freewnn"
$cvscmd checkout -kv -N -d cvstmp FreeWnn/Wnn/etc/xutoj.c FreeWnn/Wnn/include/ FreeWnn/Wnn/uum/ &&
$cvscmd checkout -kv -N -d cvstmp -l FreeWnn
rm -r cvstmp/CVS
