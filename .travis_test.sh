#!/bin/sh

set -e

track_status() {
    while true ; do
        date
        sleep 30
    done
}

die() {
    echo "$2"
    exit $1
}

build_stage0() {
    echo "Fetching stage0 files"
    cd distfiles && ./fetch.sh stage0 && cd ..
    echo "Testing Stage0 build"
    cd bootstrap/
    ./stage0.sh || exit 1

    tar czf travis_stage0_prefix.tar.gz stage0_prefix
    scp travis_stage0_prefix.tar.gz \
        staticdistro@famkaufmann.info:public_html/travis_stage0_prefix.tar.gz.tmp
    ssh staticdistro@famkaufmann.info \
        "cd public_html && mv travis_stage0_prefix.tar.gz.tmp travis_stage0_prefix.tar.gz"
}

build_stage1() {
    echo "Fetching stage1 files"
    cd distfiles && ./fetch.sh stage1 && cd ..
    echo "Testing Stage1 build"
    cd bootstrap/

    wget https://famkaufmann.info/~staticdistro/travis_stage0_prefix.tar.gz
    tar xf travis_stage0_prefix.tar.gz

    ./stage1.sh || exit 1

    tar czf travis_stage1_packages.tar.gz stage1_packages
    scp travis_stage1_packages.tar.gz \
        staticdistro@famkaufmann.info:public_html/travis_stage1_packages.tar.gz.tmp
    ssh staticdistro@famkaufmann.info \
        "cd public_html && mv travis_stage1_packages.tar.gz.tmp travis_stage1_packages.tar.gz"
}

build_stage2() {
    echo "Fetching stage2 files"
    cd distfiles && ./fetch.sh stage2 && cd ..
    echo "Testing Stage2 build"
    cd bootstrap/

    wget https://famkaufmann.info/~staticdistro/travis_stage1_packages.tar.gz
    tar xf travis_stage1_packages.tar.gz

    ./stage2.sh || exit 1

    tar czf travis_stage2_packages.tar.gz stage2_packages
    scp travis_stage2_packages.tar.gz \
        staticdistro@famkaufmann.info:public_html/travis_stage2_packages.tar.gz.tmp
    ssh staticdistro@famkaufmann.info \
        "cd public_html && mv travis_stage2_packages.tar.gz.tmp travis_stage2_packages.tar.gz"
}

mkdir -p ~/.ssh/
mv .travis_id_rsa.pub ~/.ssh/id_rsa.pub
mv .travis_id_rsa ~/.ssh/id_rsa
chmod 600 ~/.ssh/id_rsa

make -C tools
make -C tools
track_status &

# run specified tests
git show -s | grep -e '\[test.*\]' | \
    sed 's/\s\s*\[test  *\([^] ]*\) *\]/\1/' | \
    while read l ; do
      echo $l
        case $l in
            stage0)
                build_stage0
                ;;
            stage1)
                build_stage1
                ;;
            stage2)
                build_stage2
                ;;
            *)
                >&2 echo "Unknown test: $l"
                exit 1
                ;;
        esac
    done
