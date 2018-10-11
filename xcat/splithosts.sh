#!/bin/bash
[ -z "$1" -o -z "$2" ] && echo "$0 {max hosts} {xcat noderange}" && exit 1

nodels "$2" | awk -F, -v mn="$1" '
BEGIN { list=""; n=0; }
$0 !~ /^#/ && $0 !~ /^$/ {
    if(n >= mn)
    {
	print list;
	n=0;
	list="";
    }

    n++;
    if(list != "")
	list=list ",";
    list=list $1;
}
END { 
    if(list != "") print list 
}
'
