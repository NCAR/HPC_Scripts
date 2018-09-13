#/bin/bash
ruser="$1"
kuser="$2"
rhost="$3"
rcluster="$4"
rfile="$5"
rdir="$6"
rdirwhat="$7"
[ -z "$1" -o -z "$3" -o -z "$4" -o -z "$5" -o -z "$6" -o -z "$7" ] && \
    echo "$0 {user} {kerberos user|} {hsi host} {cluster} {hsi file name} {directory to tar} {what in directory to tar}" && exit 1

export PATH=/ssg/bin/backup:$PATH
. clustername

TAROPTS=

#auto detect if xattrs is supported
tar --usage|grep xattr 2>&1 >/dev/null
[ $? -eq 0 ] && TAROPTS="$TAROPTS --xattrs "

#auto detect pigs
pigz=$(which pigz 2>&1)
[ -x /usr/bin/pigz ] && pigz="/usr/bin/pigz" #prefer local pigz if it exists

#auto detect pbzip2 -- way too slow for backups
#pbzip=$(which pbzip2 2>&1)
#[ -x /usr/bin/pbzip2 ] && pbzip="/usr/bin/pbzip2" #prefer local pbzip2 if it exists

if [ ! -z "$pbzip" -a -x "$pbzip" ]  
then
    TAROPTS="$TAROPTS --use-compress-program=$pbzip " 
elif [ ! -z "$pigz" -a -x "$pigz" ] 
then
    TAROPTS="$TAROPTS --use-compress-program=$pigz " 
fi

TFILE=/tmp/tar.totals.$$
 
echo "start backup tarball: directory: $rdir contents: $rdirwhat" | logger -t backup
tar --totals $TAROPTS -C "$rdir" --numeric-owner -pcf - $rdirwhat 2> $TFILE \
    | $USE_THROTTLE \
    | tohsi.sh "$ruser" "$kuser" "$rhost" "$rcluster" "$rfile"

echo "end backup tarball: $(cat $TFILE)" | logger -t backup
unlink $TFILE 
