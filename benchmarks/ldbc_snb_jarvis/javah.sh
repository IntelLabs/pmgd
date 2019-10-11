jnidir=src/jni
incdir=$jnidir/include
driver=../ldbc_driver
for i in build/classes/main/jarvis/ldbc/*; do
    n=$(basename $i .class)
    echo $n
    javah -cp build/classes/main:$driver/target/classes -d $incdir jarvis.ldbc.$n
done
for h in $incdir/*; do
    if grep -q JNIEXPORT $h; then
        c=$jnidir/$(basename $h .h).cc
        if ! test -f $c; then
            cp $h $c
        fi
    else
        rm -f $h
    fi
done
