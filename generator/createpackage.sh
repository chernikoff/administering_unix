#!/bin/sh
fpm -t deb -s dir -n generator -v 1.1 --category "Office" -d "libc6" -d "make" /opt/generator/
