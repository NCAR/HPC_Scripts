#!/bin/sh
#BSUB -n 1
#BSUB -R "span[ptile=1]"
##BSUB -o output/out.%J
##BSUB -e output/err.%J
#BSUB -q geyser
#BSUB -W 6:00
#BSUB -P SSSG0001
#BSUB -a poe
#BSUB -network 'type=sn_all'
##BSUB -R "select[poe > 0 && nrt_windows > 0]"

RES=1280x768 
[ "$1" = "nate" ] && RES=1800x1100
[ "$1" = "nate2" ] && RES=1920x1200

export TARGET_CPU_RANGE="-1"
export LANG=en_US
export MP_PE_AFFINITY=no

export PATH=/opt/TurboVNC/bin/:/opt/VirtualGL/bin/:/ncar/opt/cuda/current/bin:$PATH
export LD_LIBRARY_PATH=/ncar/opt/cuda/current/lib64:/ncar/opt/cuda/current/lib:$LD_LIBRARY_PATH
export EXTRA_LDFLAGS="-L/ncar/opt/cuda/current/lib64 -L/ncar/opt/cuda/current/lib"

#vncserver -geometry 1280x768 -depth 16 -fg
vncserver -geometry $RES -otp -fg -nopam

grep noblank .vnc/xstartup.turbovnc || echo "
xset -dpms
xset s off
xset s noblank
gsettings set org.gnome.settings-daemon.plugins.power active false
" >> ~/.vnc/xstartup.turbovnc
