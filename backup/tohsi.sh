#/bin/bash
ruser="$1"
kuser="$2"
rhost="$3"
rcluster="$4"
rfile="$5"
[ -z "$1" -o -z "$3" -o -z "$4" -o -z "$5" ] && \
    echo "$0 {user} {kerberos user|} {hsi host} {cluster} {file name} " && exit 1

export PATH=/ssg/bin/backup:$PATH
. clustername

KLOGIN=
[ ! -z "$kuser" ] && KLOGIN="-l $kuser"

SU='/bin/bash '
[ "$ruser" != "root" ] && SU="su $ruser"

FILE="/backup365/SSG/$rcluster/$rfile"

GPGRECV="$(echo "$GPG_KEYS" | awk 'BEGIN { RS=" "} {printf "--trusted-key %s --recipient %s ", $1, $1}')"

if [ "$rhost" != "localhost" ]
then
    ssh -o compression=no -o BatchMode=yes $rhost \
	"gpg $GPGRECV --batch --output - --sign --encrypt | $SU -c '$HSI $KLOGIN \"put - : $FILE.gpg\"'"
else
    gpg $GPGRECV  --batch --output - --sign --encrypt \
	| $SU -c "$HSI $KLOGIN 'put - : $FILE.gpg'"
fi

echo "created backup: $FILE" | logger -t backup
