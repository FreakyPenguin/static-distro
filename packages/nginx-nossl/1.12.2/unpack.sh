#!/bin/sh
tar xof nginx-*.tar.*
cd nginx-*/
patch -p1 < ../no-Eflag.patch
