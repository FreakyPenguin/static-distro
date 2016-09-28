#!/bin/sh

track_status() {
    while true ; do
        date
        sleep 30
    done
}


mkdir ~/.ssh/
mv .travis_id_rsa.pub ~/.ssh/id_rsa.pub
mv .travis_id_rsa ~/.ssh/id_rsa

make -C tools
make -C tools setuid
track_status &

if [ "$TEST_OPTION" == "stage0" ] ; then
    echo "Fetching stage0 files"
    cd distfiles && ./fetch.sh stage0 && cd ..
    echo "Testing Stage0 build"
    cd bootstrap/
    ./stage0.sh || exit 1

    tar czf travis_stage0_prefix.tar.gz stage0_prefix
    scp stage0_prefix.tar.gz famkaufmann.info:public_html/
fi
