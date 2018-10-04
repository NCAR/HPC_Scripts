#!/bin/bash
xd=/root/ldap/ #config directory
ldap_user_ou="ou=accounts,ou=cheyenne,ou=unix,dc=ucar,dc=edu"
ldap_group_ou="ou=groups,ou=hpc-data,ou=unix,dc=ucar,dc=edu"
min_users=4
min_groups=1

#ldap for cluster
mkdir -p $xd
/usr/sbin/ldapprocess.pl ldaphpc.ucar.edu "$ldap_user_ou" posixAccount "$ldap_group_ou" posixGroup member '' '' 0 /etc/passwd /etc/shadow /etc/group $xd/passwd $xd/shadow $xd/group ""
[ $? -ne 0 ] && echo "Error: Problem with LDAP update" && exit 1

#make sure root exists fully
grep "^root:" $xd/shadow >/dev/null 
test $? -ne 0 && echo "root does not exist. bailing!" && exit 1
grep "^root:x:0:0:root:/root:/bin/bash$" $xd/passwd >/dev/null 
test $? -ne 0 && echo "root does not exist. bailing!" && exit 1

#make sure special users exist as good test of ldap users
for user in matthews robertsj nate stormyk tomk mail 
do
    grep "^$user:" $xd/shadow >/dev/null 
    test $? -ne 0 && echo "$user does not exist. bailing!" && exit 1
    grep "^$user:" $xd/passwd >/dev/null 
    test $? -ne 0 && echo "$user does not exist. bailing!" && exit 1
done

# make sure enough users were added
[ $(cat $xd/passwd| wc -l) -lt $(($(cat /etc/passwd|wc -l) + $min_users)) ] && echo "not enough users added. something is wrong! bailing." && exit 1
[ $(cat $xd/group| wc -l) -lt $(($(cat /etc/group|wc -l) + $min_groups)) ] && echo "not enough groups added. something is wrong! bailing." && exit 1

#promote files once sanity tests are done
cp -vf $xd/passwd /etc/passwd
cp -vf $xd/shadow /etc/shadow
cp -vf $xd/group  /etc/group 

true

