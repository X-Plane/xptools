#!/bin/sh
#USAGE: make_world_range_full <west> <south> <east> <north> <command> <dir>

west=$1
south=$2
east=$3
north=$4
cmd=$5
dst=$6

for (( tile_x = $west; tile_x <= $east; tile_x+=10 ))
do
        for (( tile_y = $south; tile_y <= $north; tile_y+=10 ))
        do
                $cmd $tile_x $tile_y $dst
        done
done
