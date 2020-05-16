#! /bin/sh

cd "$(dirname "$0")"

ssh Kevin@192.168.8.4 < "./winbuild.bat"
