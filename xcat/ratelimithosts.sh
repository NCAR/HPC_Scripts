#!/bin/bash
[ -z "$1" -o -z "$2" -o -z "$3" -o -z "$4" ] && echo "$0 {max hosts} {sleep} {hosts} {command}" && exit 1

/opt/xcat/bin/nodels "$3" | awk -F, -v mn="$1" '
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
'| while read i; 
do  
    echo "batch: $i"
    echo "start:"
    echo "$i" | xargs -i "$4" "$5" "$6" "$7" "$8" "$9" "${10}"
    echo "stop: sleep now"
    sleep $2;

    /bin/true
done

