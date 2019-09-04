#!/bin/bash
set errorCounter = 0
for i in {1..10}
do
    for i in {1..55}
    do
        echo "Command $i"
        curl "http://192.168.1.1/set_animation?v=$i" --max-time 5
        # if [ $? -ne 0 ]; then
            # errorCounter=${errorCounter} + 1
        # fi
        sleep 1
    done
done

echo "Counter Error = $errorCounter"
