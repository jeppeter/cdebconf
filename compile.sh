#! /bin/sh

_scriptfile=`readlink -f $0`
_scriptdir=`dirname $_scriptfile`

autoreconf -fi && ./configure --with-syslog-logging && make

