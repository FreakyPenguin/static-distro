#!/bin/sh

track_status() {
    while true ; do
        date
        sleep 30
    done
}

echo "Testing Stage0 build"
track_status &
cd bootstrap/
./stage0.sh || exit 0
exit 1
