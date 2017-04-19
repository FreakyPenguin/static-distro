#!/bin/sh
tar xof Python-*.tar.*
cd Python-*/
patch -p1 < ../static-modules.diff
