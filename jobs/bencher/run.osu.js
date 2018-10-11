#!/bin/bash

export RUNLOOPS=32

export OUTPUT_PATH=~/output.osu/2node.1ptile.a1/ USEPOE=yes NODES=2 PTILE=1; . jellystone.env.sh;./osu.run.sh /picnic/scratch/ssgadmin/osu-micro-benchmarks/4.4.1/ | bsub -R 'select[a1] span[ptile=1]'
export OUTPUT_PATH=~/output.osu/2node.1ptile.a2/ USEPOE=yes NODES=2 PTILE=1; . jellystone.env.sh;./osu.run.sh /picnic/scratch/ssgadmin/osu-micro-benchmarks/4.4.1/ | bsub -R 'select[a2] span[ptile=1]'

export OUTPUT_PATH=~/output.osu/2node.1ptile.js0101.js0117/ USEPOE=yes NODES=2 PTILE=1; . jellystone.env.sh;./osu.run.sh /picnic/scratch/ssgadmin/osu-micro-benchmarks/4.4.1/ | bsub -m 'js0101 js0117'

export OUTPUT_PATH=~/output.osu/8node.1ptile.a1/ USEPOE=yes NODES=8 PTILE=1; . jellystone.env.sh;./osu.run.sh /picnic/scratch/ssgadmin/osu-micro-benchmarks/4.4.1/ | bsub -R 'select[a1] span[ptile=1]'
export OUTPUT_PATH=~/output.osu/8node.32ptile.a1/ USEPOE=yes NODES=8 PTILE=32; . jellystone.env.sh;./osu.run.sh /picnic/scratch/ssgadmin/osu-micro-benchmarks/4.4.1/ | bsub -R 'select[a1] span[ptile=32]'

export OUTPUT_PATH=~/output.osu/16node.1ptile.js0101-08.js0117-24/ USEPOE=yes NODES=16 PTILE=1; . jellystone.env.sh;./osu.run.sh /picnic/scratch/ssgadmin/osu-micro-benchmarks/4.4.1/ | bsub -m "js0101 js0102 js0103 js0104 js0105 js0106 js0107 js0108 js0117 js0118 js0119 js0120 js0121 js0122 js0123 js0124"
export OUTPUT_PATH=~/output.osu/16node.32ptile.js0101-08.js0117-24/ USEPOE=yes NODES=16 PTILE=32; . jellystone.env.sh;./osu.run.sh /picnic/scratch/ssgadmin/osu-micro-benchmarks/4.4.1/ | bsub -m "js0101 js0102 js0103 js0104 js0105 js0106 js0107 js0108 js0117 js0118 js0119 js0120 js0121 js0122 js0123 js0124"

export OUTPUT_PATH=~/output.osu/32node.1ptile/ USEPOE=yes NODES=32 PTILE=1; . jellystone.env.sh;./osu.run.sh /picnic/scratch/ssgadmin/osu-micro-benchmarks/4.4.1/ | bsub 
export OUTPUT_PATH=~/output.osu/32node.32ptile/ USEPOE=yes NODES=32 PTILE=32; . jellystone.env.sh;./osu.run.sh /picnic/scratch/ssgadmin/osu-micro-benchmarks/4.4.1/ | bsub 
