#!/usr/bin/php
<?php
ini_set('display_errors', 'On');
error_reporting(E_ALL);
date_default_timezone_set('America/Denver');

if(!isset($argv[1]))
    die($argv[0] . " {ibdiscover syslog file} {asset tracker out file} {filter for only post swap events} {filter from date} {filter from date}");
$src_ibdiscover_filename = $argv[1];
$src_ibdiscover_filename = $argv[1];
$src_assettracker_filename = $argv[2];
$filterpostswap = false;
if($argv[3] == 'yes')
    $filterpostswap = true;
$filterfromtime = strtotime($argv[4]);
$filtertotime = strtotime($argv[5]); 


define("MERGEINTERVAL", 3600);#merge events within 1 hour 
#define("MERGEINTERVAL", 24*60*60);#merge events within 1 day

$cablecount = 0;
$cables = array();
/// cable port names => cable id dictionary
$cableportnames = array(); 
$devices = array(); 

$cablecount = 0;
$cables = array();
/// cable port names => cable id dictionary
$cableportnames = array(); 
$devices = array(); 
$portnames = array();

/**
 * @brief label port to port name
 */
function lptopn($label, $port)
{
    GLOBAL $portnames;
    $n =  $label . "/P". $port;
    $portnames[$n] = array(
	'label' => $label,
	'leaf' => NULL,
	'port' => $port
    );
    return $n;
}

/**
 * @brief label leaf port to port name
 */
function llptopn($label, $leaf, $port)
{
    GLOBAL $portnames;
    $n = $label .'/L'. $leaf .'/P'. $port;
    $portnames[$n] = array(
	'label' => $label,
	'leaf' => $leaf,
	'port' => $port
    );                 
    return $n;
}
                             
/**
 * @brief get cable id
 * uses the port names and finds the correct cable
 * or registers it
 */
function getcable($portname1, $portname2)
{
    GLOBAL $devices,$cablecount,$cableportnames,$cables,$portnames;

    #find is already known
    if(isset($cableportnames[$portname1][$portname2]))
	return $cableportnames[$portname1][$portname2];
    if(isset($cableportnames[$portname2][$portname1]))
	return $cableportnames[$portname2][$portname1];

    #new cable found
    ++$cablecount;
    $cableportnames[$portname1][$portname2] = $cablecount;
    $cableportnames[$portname2][$portname1] = $cablecount;
    $cables[$cablecount] = array(
	'ports' => array($portname1, $portname2),
	'name' => $portname1 .' <-> '. $portname2,
	'events' => array(), #array of each event type
 	'incidents' => array(), #array of each incident
	'dumped' => false, #cable dumped to output?
	'timeslow' => 0, #accumlated time in slow incidents
 	'counts' => array( #count of each incident type
	    'cable_swapped' => 0,
	    'slow_link' => 0
	)
    );

    #add to device dictionary
    $devices[$portnames[$portname1]['label']][] = $cablecount;
    $devices[$portnames[$portname2]['label']][] = $cablecount;

    return $cablecount;
}

/**
 * @brief mark cable timeline with stamp (sort into windows later)
 * @param type type of event
 * @param what what event is.... FDR10 vs QDR etc
 */
function mark_timeline($cid, $timestamp, $type, $what)
{
    GLOBAL $cablecount,$cableportnames,$cables,$filterfromtime,$filtertotime;
    if(!isset($cables[$cid]))
	die("unknown cable $cid");

    if(!($timestamp >= $filterfromtime && $timestamp <= $filtertotime))
	return;
    
    $merged = false;
    if(!isset($cables[$cid]['events'][$type][$what]))
	$cables[$cid]['events'][$type][$what] = array();
    
    $cables[$cid]['events'][$type][$what][] = $timestamp;
}

{ #process the ibdiscover source file of syslogs
    $srcf = fopen($src_ibdiscover_filename, 'r');
    if(!is_resource($srcf))
	die('unable to open src file');

    while(!feof($srcf))
    {
	$line = fgets($srcf);
	#Jan  1 01:01:36 ysmgt2 ibhealthreport: ibnetdiscover: 'MF0;ys72ib1:SXX536/L28/U1'(389/2) <->  'MF0;ys59ib3:SX60XX/U1'(5077/24) : link speed = FDR10 !=  FDR
	if(preg_match('/^^((\S+)\s+\S+\s+\d+:\d+:\d+)\s+(\w+)\s+ibhealthreport:\s+ibnetdiscover:\s+\'(MF0;(\S+)(:\S+\/L(\d+)\/U\d+|:\S+\/U\d+)|(\S+)\s+[hH][Cc][aA]\S+)\'\(\d+\/(\d+)\)\s+<\->\s+\'(MF0;(\S+)(:\S+\/L(\d+)\/U\d+|:\S+\/U\d+)|(\S+)\s+[hH][Cc][aA]\S+)\'\(\d+\/(\d+)\)\s+:\slink\s+speed\s+=\s+([\w\d]+)\s+!=\s+([\w\d]+)/', $line, $match))
	{
	    #var_dump($match); continue;

            #hack: year not given by syslog data
	    $year = 2013;
	    if($match[2] == "Nov" || $match[2] == "Dec")
		$year = 2012;
	    $timestamp = strtotime($match[1]." ".$year);

	    $pn1 = FALSE;
	    $pn2 = FALSE;

	    if($match[8] != "") #hca
		$pn1 = lptopn($match[8],$match[9]); 
	    elseif($match[7] == "") #tor
		$pn1 = lptopn($match[5],$match[9]); 
	    else #orca
		$pn1 = llptopn($match[5],$match[7],$match[9]); 

	    if($match[14] != "") #hca
		$pn2 = lptopn($match[14],$match[15]); 
	    elseif($match[13] == "") #tor
		$pn2 = lptopn($match[11],$match[15]); 
	    else #orca
		$pn2 = llptopn($match[11],$match[13],$match[15]); 
 
	    if($pn1 !== FALSE)
	    {
		$cid = getcable($pn1, $pn2);
		#var_dump(array( $pn1, $pn2, $cid, $timestamp, 'slow link', $match[17]));
		mark_timeline($cid, $timestamp, 'slow_link', $match[16]);
	    } else var_dump($match);
	} #else var_dump($line);
    }

    fclose($srcf);
}
if($src_assettracker_filename != ""){ #process the asset tracker file 
    $srcf = fopen($src_assettracker_filename, 'r');
    if(!is_resource($srcf))
	die('unable to open src file');

    while(!feof($srcf))
    {
	$line = fgets($srcf);
	if(preg_match('/^(\S+)\s+(\S+)\s+(\S+\/\S+\/\S+)\s+((\S+):\S+)\s+[Ll][iI][dD]:\s*([lL](\d+)|\S*|)\s*([Pp][Cc][Ii]\s+[Pp][oO][rR][tT]|[Pp][oO][rR][tT]):\s+((\d+)|\S+\/L(\d+)\/U\d+:\(\d+\/(\d+)\))\s+(<>|<->)\s+((\S*):\S+)\s+([Pp][Cc][Ii][\s\S]*\s+[Pp][oO][rR][tT]|[Pp][oO][rR][tT]):\s+((\d+)|\(\d+\/(\d+))/', $line, $match))
	{
	    #var_dump($match);

 	    $pn1 = FALSE;
	    $pn2 = FALSE;

	    $timestamp = strtotime($match[3] . ' 11:59pm');
 
	    if($match[7] == "") #tor
	    {
		if($match[11] == "") 
		{
		    if($match[12] == "") 
			$pn1 = lptopn($match[5],$match[9]); 
		    else
			$pn1 = lptopn($match[5],$match[12]); 
		}
		else
		    $pn1 = llptopn($match[5],$match[11],$match[12]); 
	    }
	    else #orca
		$pn1 = llptopn($match[5],$match[7],$match[9]); 

	    if(isset($match[19])) #hca
		$pn2 = lptopn($match[15],$match[19]); 
	    else
	    {
		if($match[18] == "")
		    $pn2 = lptopn($match[15],$match[17]); 
		else
		    $pn2 = lptopn($match[15],$match[18]); 
	    }

 	    if($pn1 !== FALSE)
	    {
		$cid = getcable($pn1, $pn2);
		#var_dump(array( $pn1, $pn2, $cid, $timestamp, 'slow link', $match[17]));
		mark_timeline($cid, $timestamp, 'cable_swapped', 'asset_tracker');
	    } else var_dump($match);
	}else var_dump($line);
    }

    fclose($srcf);
} 

#var_dump($cables);die;
{ #use timestamps to merge into incident windows
    foreach($cables as $cid => $cable)
	foreach($cable['events'] as $type => $events)
	    foreach($events as $what => $stamps)
	    {
		#$cables[$cid]['events'][$type][$what][] = $timestamp;
		if(!isset($cables[$cid]['incidents'][$type][$what]))
		    $cables[$cid]['incidents'][$type][$what] = array();
	    
		asort($stamps);

		#find an existing window and expand it if required for given what/type
		foreach($stamps as $timestamp)
		{
		    $merged = false;

		    foreach($cables[$cid]['incidents'][$type][$what] as &$et)
			#is timestamp in merge window
			if($et['start'] - constant('MERGEINTERVAL') <= $timestamp && $et['stop'] + constant('MERGEINTERVAL') >= $timestamp) 
			{
			    #set start/stop to new timestamp if timestamp is <> than one of them
			    if($timestamp > $et['stop'])
				$et['stop'] = $timestamp;
			    elseif($timestamp < $et['start'])
				$et['start'] = $timestamp;

			    $merged = true;
			    break;
			}

		    #event at given time not found
		    if(!$merged)
			$cables[$cid]['incidents'][$type][$what][] = array(
			    'start' => $timestamp,
			    'stop' => $timestamp
			);  
		}
	    }
}

function dump_cable($cid, $offset = "", $filtereventpostswap, &$cableswaps)
{
    GLOBAL $cables;
    $cable =& $cables[$cid];

    if(!isset($cable['incidents'])) return "";

    $st = array(); #make array to sort cronologically
    foreach($cable['incidents'] as $type => $te)
	foreach($te as $what => $tw)
	    foreach($tw as $key => $inc)
	    {
		if(!$cable['dumped'])
		{
		    if(!isset($cable['counts'][$type]))
			$cable['counts'][$type] = 0;
		    ++$cable['counts'][$type];

		    $cable['timeslow'] += $tw[$key]['stop'] - $tw[$key]['start'];
		}

		$st[$inc['stop']][] = "\tevent: ".$type.' why:'.$what.' time window: '.date(DATE_ISO8601, $tw[$key]['start']).' to '.date(DATE_ISO8601, $tw[$key]['stop'])."\n";
	    }
    
    $swaptime = 0;
    foreach($cable['incidents'] as $type => $te)
	foreach($te as $what => $tw)
	    foreach($tw as $key => $inc)
		if($type == 'cable_swapped' && ($swaptime == 0 || $swaptime >= $tw[$key]['start']))
		{
		    ++$cableswaps;
		    $swaptime = $tw[$key]['start'];
		}

    $postswap = false;
    if($swaptime != 0)
	foreach($cable['incidents'] as $type => $te)
	    foreach($te as $what => $tw)
		foreach($tw as $key => $inc)
		    if($swaptime < $tw[$key]['stop'])
			$postswap = true;

    ksort($st);

    if(!$filtereventpostswap || $postswap)
    {
	$cable['dumped'] = true;

	$output = "";
	$output .= $offset . 'cable: '. $cable['name'] . "\n";
	foreach($st as $e)
	    foreach($e as $et)
		$output .= $offset . $et; 
	return $output;
    } else return "";
}

#dump the cable events
#foreach($cables as $cid => $cable)
#    dump_cable($cid);

$dumpedcables = array();
$totalcables = 0;
$totalcableswaped = 0;

ksort($devices);
foreach($devices as $device => $dev)
{
    $str = "";
    foreach($dev as $cid)
    {
	$cableswaps = 0;
	$cdump = dump_cable($cid, "\t", $filterpostswap, $cableswaps);
	$str .= $cdump;

	if($cdump != "" && !in_array($cid, $dumpedcables))
	{
	    $totalcableswaped += $cableswaps;
	    ++$totalcables;
	    $dumpedcables[] = $cid;
	}
    }

    if($str != "")
	echo 'Device: '.$device."\n". $str;
}

echo "Total Cables Dumped: $totalcables\n";
echo "Total Cables Swapped: $totalcableswaped\n";




#create histogram of cables based on counts
{
    $cablecounts = array();
    $cabletime = array();
    foreach($cables as $cid => $cable)
	if($cable['dumped'] && !empty($cable['incidents']))
	{
	    $cabletime[] = $cable['timeslow'] / 3600; #convert to hours

	    foreach($cable['counts'] as $type => $count)
	    {
		if(!isset($cablecounts[$type][$count]))
		    $cablecounts[$type][$count] = 0;

		++$cablecounts[$type][$count];
	    }

	    if($cable['counts']['slow_link'] > 0)
	    {
		$type = 'slow_and_swapped';
		if(!isset($cablecounts[$type][$cable['counts']['cable_swapped']]))
		    $cablecounts[$type][$cable['counts']['cable_swapped']] = 0;

		++$cablecounts[$type][$cable['counts']['cable_swapped']]; 
	    }
	}
	
    var_dump($cablecounts);
    asort($cabletime);
    echo 'Cable Slow Times: '. implode(',',array_reverse($cabletime, true))."\n";
    foreach($cablecounts as $type => $tc)
    {
	echo $type.' keys: '. implode(',',array_keys($tc))."\n";
	echo $type.': '. implode(',',$tc)."\n";
    }
    
}
?>
