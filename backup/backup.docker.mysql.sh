#/bin/bash
[ -z "$1" -o -z "$2" -o -z "$4" ] && echo "$0 {docker container} {user} {kerberos login} {hsi host} [nolock]" && exit 1 
export PATH=/ssg/bin/backup:$PATH
. $(dirname $0)/clustername

if [ "$5" != "nolock" ]
then
    #lock and unshare to avoid clobbering the mount namespace
    flock -xn /var/run/backup_docker_mysql_lock -c "$0 \"$1\" \"$2\" \"$3\" \"$4\" nolock"
    if [ $? -ne 0 ]
    then
        echo "unable to obtain lock. bailing!" 
        exit 5
    fi
    exit 0
fi

container="$1"
ruser="$2"
kuser="$3"
rhost="$4"

day=$(date +%m%d%y.%s)
host=$(hostname -s)

echo "Backup mysql from docker $container"
docker exec $container mysqldump -u root --create-options --tz-utc --add-drop-trigger --hex-blob --add-drop-database --add-drop-table --all-tablespaces --all-databases | tohsi.sh "$ruser" "$kuser" "$rhost" "$cluster" "$host.docker.${container}.partial.mysql.${day}.tgz"
docker exec $container mysqldump -u root --events --routines --triggers --create-options --tz-utc --add-drop-trigger --hex-blob --add-drop-database --add-drop-table --all-tablespaces --all-databases | tohsi.sh "$ruser" "$kuser" "$rhost" "$cluster" "$host.docker.${container}.mysql.${day}.tgz"

