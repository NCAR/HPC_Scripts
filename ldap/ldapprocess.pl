#!/usr/bin/perl
$|++;
use warnings;
use strict;
use Net::LDAP;

die "process {ldap server} {user base dn} {user class} {group base dn} {group class} {group member field name} {machine bluefire/glade} {explicit user list} {1/0 filter users by assigned groups} {input passwd} {input shadow} {input group} {output passwd} {output shadow} {output group} {base path to home dir}" if $#ARGV != 15;

my $i = 0;
my $ldap_server = $ARGV[$i++]; 
my $userbase = $ARGV[$i++];  #"ou=accounts,ou=unix,dc=ucar,dc=edu"
my $userclass = $ARGV[$i++]; #posixAccount
my $groupbase = $ARGV[$i++];  #"ou=groups,ou=unix,dc=ucar,dc=edu"
my $groupclass = $ARGV[$i++]; #posixGroup
my $groupmemberfield = $ARGV[$i++]; #member or uniqueMember
my $machine = $ARGV[$i++]; 
my $euserlist = $ARGV[$i++]; 
my $groupfilter = $ARGV[$i++]; 
my $input_passwd = $ARGV[$i++]; 
my $input_shadow = $ARGV[$i++]; 
my $input_group = $ARGV[$i++]; 
my $output_passwd = $ARGV[$i++]; 
my $output_shadow = $ARGV[$i++]; 
my $output_group = $ARGV[$i++]; 
my $home_base = $ARGV[$i++]; #e.g. /home

my %users = (); #list of users   user name => ( props )
my %groups = (); #list of groups   group name => { gid = str, password => str, users => { usern => 1 } }
my %group_ids = (); #list of group ids   gid => name  used to catch changed gids
my %user_ids = (); #list of user ids   gid => name  used to catch changed gids
my %user_pass = (); #list of user passwords uid => password  used to rebuild shadow
my %filterusers = (); #list of user assigned to groups for machine uid => 1

{ #extract list of all valid users on system
	open(PASSWD, '<', $input_passwd) or die "Error: Unable to open passwd for reading, $!";
	while(my $line = <PASSWD> )
	{
		chomp($line);
		if($line !~ m/^\s*#/)
		{
			my ($login, $passwd, $uid, $gid, $gecos, $home, $shell) = split(/:/, $line);
			$users{$login} = {
			    uid => $login,
			    uidNumber => $uid,
			    gidNumber => $gid,
			    gecos => $gecos,
			    homeDirectory => $home,
			    loginShell => $shell
			};
			$user_ids{$uid} = $login;
		}
	}
	close(PASSWD) or die "Error: Unable to close passwd $!";
}

{ #extract list of all user passwords
	open(SHADOW, '<', $input_shadow) or die "Error: Unable to open passwd for reading, $!";
	while(my $line = <SHADOW> )
	{
		chomp($line);
		if($line !~ m/^\s*#/)
		{
			my ($login, $passwd) = split(/:/, $line);
			$user_pass{$login} = $passwd;
		}
	}
	close(SHADOW) or die "Error: Unable to close passwd $!";
} 

{ #extract list of all valid groups on system
	open(GROUP, '<', $input_group) or die "Error: Unable to open group file for reading, $!";
	while(my $line = <GROUP> )
	{
		chomp($line);
		if($line !~ m/^\s*#/)
		{
			my ($name, $password, $gid, $userslist) = split(/:/, $line);
 			$groups{$name} = {
			    cn => $name,
			    gidNumber => $gid
			};      

			my %members = ();
			foreach my $user ( split(/,/, $userslist) )
			{
			    #$members{$user} = 1 if exists $users{$user};
			    $members{$user} = 1;
			}
			$groups{$name}{$groupmemberfield} = {%members}; 
			$group_ids{$gid} = $name;
		}
	}
	close(GROUP) or die "Error: Unable to close group file $!";
} 

my $ldap = Net::LDAP->new( $ldap_server ) or die "$@";
$ldap->bind or die "$@";

if($euserlist ne "")
{ #explicit user list provided
    open(EUSERS, '<', $euserlist) or die "Error: Unable to open user file for reading, $!";
    while(my $user = <EUSERS> )
    {
	    my $shell = 1;
	    chomp($user);
	    if($user =~ m/(.+):(.+)/)
	    {
		$user = $1;
		$shell = $2
	    }
	    $shell = $filterusers{$user} if exists $filterusers{$user} && $filterusers{$user} ne 1;
	    $filterusers{$user} = $shell;
    }
    close(EUSERS) or die "Error: Unable to close user file $!"; 
}
elsif($groupfilter)
{ #query the groups to get valid users for machine
    my $filter = "(objectClass=$groupclass)";
    $filter = "(&(objectClass=$groupclass)(x-ucar-tag=$machine))" if $machine ne '';

    my $entries = $ldap->search(
	base   => $groupbase,
        filter => $filter,
	attrs  => [ 'cn', $groupmemberfield  ]
    ) or die "$@";

    while(my $entry = $entries->pop_entry())
    {
	my %members = ();

        #load and parse members
	foreach my $m ( @{ $entry->get_value($groupmemberfield, asref => 1) } )
	{
	    foreach my $tag ( split(/,/, $m) )
	    {
		my ($key, $value) = split(/=/, $tag);
		$filterusers{$value} = 1;
	    }
	} 
    }
}

{ #query the users
    my $filter = "(&(objectClass=$userclass)(host=$machine))";
    $filter = "(objectClass=$userclass)" if $machine eq '' ||  scalar keys %filterusers; #get all users when user filter is active since its faster to query all then send one query per user

    my $entries = $ldap->search(
	base   => $userbase,
	filter => $filter,
	attrs  => [ 'uid', 'uidNumber', 'gidNumber', 'gecos', 'homeDirectory', 'loginshell' ]
    ) or die "$@";

    while(my $entry = $entries->pop_entry())
    {
	#$entry->dump; 
	my %props = ();

	foreach my $attr ( $entry->attributes ) 
	{
	    $props{$attr} = $entry->get_value($attr);
	}

	#skip users that are not in valid group if user filter is active
	next if scalar keys %filterusers && ! exists $filterusers{$props{'uid'}};

	#override for user shells not in tpl files
	if(exists $filterusers{$props{'uid'}} && $filterusers{$props{'uid'}} ne "1")
	{
	    $props{'loginShell'} = $filterusers{$props{'uid'}};
	}

	if($props{'uidNumber'} >= 1000) #safety to keep system users untouched
	{
	    if(exists $user_ids{$props{'uidNumber'}} && $user_ids{$props{'uidNumber'}} ne $props{'uid'})
	    {
		warn "user ".$props{'uid'}."[".$props{'uidNumber'}."] duplicate of existing user ".$user_ids{$props{'uidNumber'}}. "[".$props{'uidNumber'}."]\n";
		$props{'uid'} = $user_ids{$props{'uidNumber'}};
	    }

	    $users{$props{'uid'}} = {%props} unless exists $user_ids{$props{'uidNumber'}};
	    $user_ids{$props{'uidNumber'}} = $props{'uid'};
	    if($home_base ne "")
	    {
	  	$users{$props{'uid'}}{'homeDirectory'} = $home_base.'/'.$props{'uid'}
	    }
	}
    }
}

{ #query the groups
    my $filter = "(objectClass=$groupclass)";
    $filter = "(&(objectClass=$groupclass)(x-ucar-tag=$machine))" if $machine ne '';
 
    my $entries = $ldap->search(
	base   => $groupbase,
	filter => $filter,
	attrs  => [ 'cn', $groupmemberfield, 'gidNumber' ]
    ) or die "$@";

    while(my $entry = $entries->pop_entry())
    {
	my %props = ();
	my %members = ();

	foreach my $attr ( $entry->attributes ) 
	{
	    if($attr ne $groupmemberfield)
	    {
		#$props{$attr} = $entry->get_value($attr, asref => 1);
		$props{$attr} = $entry->get_value($attr);
	    }
	}

	#load and parse members
	if($entry->exists($groupmemberfield, asref => 1))
	{
	    foreach my $m ( @{ $entry->get_value($groupmemberfield, asref => 1) } )
	    {
		foreach my $tag ( split(/,/, $m) )
		{
		    my ($key, $value) = split(/=/, $tag);
		    $members{$value} = 1 if $key eq 'uid' && exists $users{$value};
		}
	    }
	}
	$props{$groupmemberfield} = {%members};

	if($props{'gidNumber'} >= 1000) #safety to keep system groups untouched
	{
	    #handle existing group members
	    if(exists $groups{$props{'cn'}})
	    {
		my $rgrp = $groups{$props{'cn'}};
		my %grp = %$rgrp;
		if(! exists $grp{$groupmemberfield} || scalar(keys %{$grp{$groupmemberfield}}) <= 0)
		{
		    warn "Group ".$props{'cn'}." has no members.";
		    $props{$groupmemberfield} = ();
		}
		else #has members listed
		{
		    foreach my $m ( keys %{$grp{$groupmemberfield}} )
		    {
			$props{$groupmemberfield}{$m} = 1;
		    }
		}
	    }
	    if(exists $group_ids{$props{'gidNumber'}} && $group_ids{$props{'gidNumber'}} ne $props{'cn'})
	    {
		warn "group ".$props{'cn'}."[".$props{'gidNumber'}."] duplicate of existing group ".$group_ids{$props{'gidNumber'}}. "[".$props{'gidNumber'}."]\n";
		$props{'cn'} = $group_ids{$props{'gidNumber'}};
	    }
	    $group_ids{$props{'gidNumber'}} = $props{'cn'};
	    $groups{$props{'cn'}} = { %props };
	}
    }
}

$ldap->unbind or die "$@";

#Populate the "sysncar" group with all system users
#
if (not exists $groups{'sysncar'}) 
{
	die "Error: sysncar group is missing please ask USS why"
}
#ignore what we got from LDAP
%{$groups{'sysncar'}{'member'}} = ();
%{$groups{'users'}{'member'}} = ();

foreach my $u (keys %users)
{
	#just assume that all users have 1000 < uid 
	if($users{$u}{'uidNumber'} >= 1000)
	{
		$groups{'users'}{'member'}{$u} = 1;
	}
	else #make list of system users
	{
		$groups{'sysncar'}{'member'}{$u} = 1;
	}
}

{ #write updated passwd file
	open(PASSWD, '>', $output_passwd) or die "Error: Unable to open new passwd file output_group for writing $!";
	open(SHADOW, '>', $output_shadow) or die "Error: Unable to open new shadow file output_group for writing $!";
	foreach my $name (sort(keys(%users)))	
	{
		my $passwd = '*';
		$passwd = $user_pass{$name} if exists $user_pass{$name};  

		my %user = %{$users{$name}};

		$user{'gecos'} = "" unless defined $user{'gecos'};
		$user{'loginShell'} = "/sbin/nologin" unless defined $user{'loginShell'};
		$user{'homeDirectory'} = "/tmp/" unless defined $user{'homeDirectory'};
		chomp($user{'gecos'});
		print PASSWD join(':', $name, 'x', $user{'uidNumber'}, $user{'gidNumber'}, $user{'gecos'}, $user{'homeDirectory'}, $user{'loginShell'} ) ."\n";
		print SHADOW $name.":".$passwd.":::::::\n";
	}
	close(PASSWD) or die "Error: $!";
	close(SHADOW) or die "Error: $!";
} 
{ #write updated group file
	#my $maxgsize = 1024;
	my $maxgsize = 300;
	open(GROUP, '>', $output_group) or die "Error: Unable to open new group file output_group for writing $!";
	foreach my $name (sort(keys(%groups)))	
	{
	    my $groupoffset = 0;
	    my @members = sort(keys(%{$groups{$name}{"member"}}));
	    
	    do {
		#split the group members to max at 1024k per line
		my $strname = $name;
		$strname = $name .'-'. $groupoffset if $groupoffset > 0; 
		my $prestr = $strname .":x:". $groups{$name}{"gidNumber"} .":";

		my $mstr = "";
		while(@members)
		{
		    my $member = shift(@members);
                    if(length($prestr) + length($mstr) + length($member) + 1 < $maxgsize)
		    {
			$mstr .= "," if length($mstr) > 0;
			$mstr .= $member;
		    }
		    else #too long
		    {
			unshift(@members, $member);
			last;
		    }
		}

		print GROUP $prestr . $mstr ."\n";
		++$groupoffset;
	    } while(@members) ;
	}
	close(GROUP) or die "Error: $!";
}
