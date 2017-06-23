#!/bin/bash
#if you edited this document at windows,you should input the follow command in vi or vim
#:set fileformat=unix
if [ $# -lt 5 ];then
    echo "usage: $0 ROLE NODE_URI NODE_PORT PS_ROOT_URI PS_ROOT_PORT"
    #program_bin [args...] 
    exit -1;
fi

export ROLE=$1
shift
export NODE_URI=$1
shift
export NODE_PORT=$1
shift
export PS_ROOT_URI=$1
shift
export PS_ROOT_PORT=$1
#shift
#bin=$1
#shift
#arg="$@"

./main


wait
