#/bin/bash
ruser="$1"
kuser=
rhost="$2"
[ -z "$1" -o -z "$2" ] && echo "$0 {user} {hsi host} [nolock] " && exit 1 

if [ "$3" != "nolock" ]
then
    flock -xn /var/run/backup_lock -c "$0 \"$1\" \"$2\" nolock"
    if [ $? -ne 0 ]
    then
        echo "unable to obtain lock. bailing!" 
        exit 5
    fi
    exit 0
fi

export PATH=/ssg/stargate-bin/backup/:/ssg/bin/backup/:/ssg/bin/:$PATH
. clustername 
 
day=$(date +%m%d%y.%s)
host=$(hostname -s)

docker ps --format "{{.Image}}" |  while read container
do
    ionice -c3 nice docker export "$container" | ionice -c3 nice tohsi.sh "$ruser" "$kuser" "$rhost" "$cluster" "$host.docker.${container}.${day}.tgz"
done

