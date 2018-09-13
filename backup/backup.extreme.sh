#!/bin/bash
ruser="$1"
kuser=
rhost="$2"
[ -z "$1" ] && echo "$0 {user} {hsi host} [nolock] " && exit 1 

if [ "$3" != "nolock" ]
then
    flock -xn /var/run/backup_extreme_sw_ssg_lock -c "$0 \"$1\" \"$2\" nolock"
    if [ $? -ne 0 ]
    then
        echo "unable to obtain lock. bailing!" 
        exit 5
    fi
    exit 0
fi

export PATH=/ssg/bin/backup/:/ssg/bin/:$PATH
cd /ssg/bin/backup/
. clustername 

day=$(date +%m%d%y.%s)
host=$(hostname -s)

function pull() {
    SW=$1
    [ -f /tftpboot/$SW-config.xsf ] && unlink /tftpboot/$SW-config.xsf
    touch /tftpboot/$SW-config.xsf
    chmod 0777 /tftpboot/$SW-config.xsf
    backup.extreme.telnet.tftp.config.py $SW
}

pull mgmtsw0
if [ "$host" = "chmgt" ] 
then
	pull mgmtsw1
	pull mgmtsw2
	pull mgmtsw3
	pull mgmtsw4
fi

exec backup.tarhsi.sh "$ruser" "$kuser" "$rhost" "$cluster" "$host.extreme.${day}.tgz" "/tftpboot/" "*-config.xsf"
