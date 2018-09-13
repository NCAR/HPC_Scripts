#/bin/bash
ruser="$1"
kuser="$2"
rhost="$3"
[ -z "$1" -o -z "$3" ] && echo "$0 {user} {kerberos login} {hsi host} [nolock] [rprivate]" && exit 1 

if [ "$4" != "nolock" ]
then
    #lock and unshare to avoid clobbering the mount namespace
    flock -xn /var/run/backup_lock -c "unshare -m $0 \"$1\" \"$2\" \"$3\" nolock rprivate"
    if [ $? -ne 0 ]
    then
        echo "unable to obtain lock. bailing!" 
        exit 5
    fi
    exit 0
fi

if [ "$5" = "rprivate" ]
then
    #unshare -m needs rprivate too to work correctly
    mount --make-rprivate /
fi

export PATH=/ssg/bin/backup:$PATH
. $(dirname "$(readlink -e "$0")")/clustername

day=$(date +%m%d%y.%s)
host=$(hostname -s)
mntroot=/mnt/root

bdir=/ssg/tmp/backup/$host.$$/

mkdir -p $bdir
chmod 0755 $bdir
chown root:root $bdir

test -x /opt/sgi/sbin/discover && \
(
    echo "Backup SMC" 
    xdir=$bdir/smc
    mkdir -p $xdir
    /opt/sgi/sbin/dbdump --backup $xdir
    /opt/sgi/sbin/discover --show-configfile --image --kernel --bmc-info --kernel-parameters > $xdir/discover.log
    cp /var/opt/sgi/lib/blademond/slot_map.* $xdir/
    backup.tarhsi.sh "$ruser"  "$kuser" "$rhost" "$cluster" "$host.smc.${day}.tgz" "$xdir" "."
)
test -x /opt/xcat/sbin/dumpxCATdb && \
(
    echo "Backup Xcat" 
    xdir=$bdir/xcat
    dumpxCATdb -p $xdir
    backup.tarhsi.sh "$ruser"  "$kuser" "$rhost" "$cluster" "$host.xcat.${day}.tgz" "$xdir" "."
)
[ -f /usr/lib/systemd/system/mysql.service -o -f /etc/init.d/mysqld -o -f /etc/systemd/system/multi-user.target.wants/mariadb.service ] && which mysqldump 2>&1 >/dev/null && \
(
    echo "Backup mysql"
    mfile=mysql-${day}.sql
    mysqldump -u root --events --routines --triggers --create-options --tz-utc --add-drop-trigger --hex-blob --add-drop-database --add-drop-table --all-tablespaces --all-databases  > $bdir/$mfile
    backup.tarhsi.sh "$ruser" "$kuser" "$rhost" "$cluster" "$host.mysql.${day}.tgz" "$bdir" "$mfile"
)
[ -f /opt/pbs/default/sbin/pbs_server.bin ] && ( #detect PBS server
    echo "Backup PBS postgres DB"
    source /etc/profile.d/pbs.sh
    xdir=$bdir/postgres
    mkdir -p $xdir
    export PGPASSWORD=PASSWORD_ENTERED_HERE LD_LIBRARY_PATH=/opt/pbs/default/pgsql/lib:$LD_LIBRARY_PATH 
    /opt/pbs/default/pgsql/bin/pg_dumpall -U pbsdata  -p 15007 > $xdir/postgres.sql
    for i in server queue node resource hook; do qmgr -c "print $i @default"; done > $xdir/qmgr.conf
    backup.tarhsi.sh "$ruser"  "$kuser" "$rhost" "$cluster" "$host.pbs.postgres.${day}.tgz" "$xdir" "."
)

#cleanup backup tmp
rm -Rf /ssg/tmp/backup/$host.$$/ 

mkdir -p $mntroot/
mnts="$(cat /proc/mounts)"
echo "$mnts" | awk '!x[$0]++' | while read src dst type opt opt2 opt3
do
    echo "$src" | /bin/grep "^/dev/" >/dev/null
    isdev=$?
    echo "$dst" | /bin/grep "^$mntroot" >/dev/null
    isbmnt=$?
    [ ! -z "$SKIP_MOUNTS" ] && echo "$src" | /bin/grep -E "$SKIP_MOUNTS" >/dev/null && continue
    if [ $isdev -eq 0 -a $isbmnt -ne 0 ] 
    then
	if [ "$type" = "ext4" -o "$type" = "ext3" -o "$type" = "vfat" -o "$type" = "xfs" -o "$type" = "btrfs" ]
	then
	    mount --bind -o ro $dst $mntroot/$dst 2>&1 |grep -v 'seems to be mounted read-write'
	    mount -o bind,remount,ro $dst $mntroot/$dst
	fi
    fi
done

#detect if lxc containers are present
if [ -x /usr/bin/lxc-ls -a -d /var/lib/lxc/ ]
then
    mkdir -p /var/lib/lxc/.snapshots

    #mask mounts using tmpfs to avoid big bmr getting the lxc containers info
    /usr/bin/lxc-ls -1 | while read mnt
    do
	backup_simple="TRUE"
	which btrfs 2>&1 >/dev/null
	if [ $? -eq 0 ]
	then
	    #if it is btrfs then use a snapshot
	    btrfs subvolume list /var/lib/lxc/$mnt 2>&1 >/dev/null
	    if [ $? -eq 0 ]
	    then #lxc inside of btrfs
		#delete any old snapshots
		btrfs subvolume delete /var/lib/lxc/.snapshots/$mnt 2>/dev/null
		touch "/var/lib/lxc/.snapshots/BACKUP BMR DIRECTORY. DO NOT CHANGE"

		#create snapshot
		btrfs subvolume snapshot -r /var/lib/lxc/$mnt /var/lib/lxc/.snapshots/
		if [ $? -eq 0 ]
		then
		    echo "snapshot backup of lxc container: $mnt"
		    backup.tarhsi.sh "$ruser" "$kuser" "$rhost" "$cluster" "lxc.$mnt.$host.bmr.${day}.tgz" "/var/lib/lxc/.snapshots/" "$mnt" 
		    backup_simple="FALSE"
		fi

		btrfs subvolume delete /var/lib/lxc/.snapshots/$mnt
	    fi

	    rmdir /var/lib/lxc/.snapshots/$mnt 2>/dev/null
	fi

	if [ "$backup_simple" = "TRUE" ]
	then
	    #create simple backup of container
	    echo "simple backup of lxc container: $mnt"
	    backup.tarhsi.sh "$ruser" "$kuser" "$rhost" "$cluster" "lxc.$mnt.$host.bmr.${day}.tgz" "/mnt/root/var/lib/lxc/" "$mnt" 
	fi

	mount -t tmpfs tmpfs "/var/lib/lxc/$mnt"
	echo "masking lxc container: $mnt"
    done

    mount -t tmpfs tmpfs "/var/lib/lxc/.snapshots"
fi

findmnt -R
backup.tarhsi.sh "$ruser" "$kuser" "$rhost" "$cluster" "$host.bmr.${day}.tgz" "/mnt/root/" "." 

if [ -x /usr/bin/lxc-ls -a -d /var/lib/lxc/ ]
then
    /usr/bin/lxc-ls -1 | while read mnt
    do
	#undo the mounts
	umount "/var/lib/lxc/$mnt" || umount -l "/var/lib/lxc/$mnt"
    done

    umount /var/lib/lxc/.snapshots
fi

awk '!x[$0]++' < /proc/mounts | tac | while read src dst type opt opt2 opt3
do
    echo "$dst" | /bin/grep "^$mntroot" >/dev/null
    if [ $? -eq 0 ]
    then
	if [ "$type" = "ext4" -o "$type" = "ext3" -o "$type" = "vfat" -o "$type" = "xfs" -o "$type" = "btrfs" ]
	then
	    umount -l $dst
	fi
    fi
done

rmdir /mnt/root
