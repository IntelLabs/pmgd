set -e
data=${1:?}
graph=${2:?}
jarvis_path=${3:?}
jarvis_tools=$jarvis_path/tools

rm -fr $graph
#$jarvis_tools/mkgraph $graph \
# SF30:
#$jarvis_tools/mkgraph $graph --node-table-size 0x200000000 --edge-table-size 0x500000000 --allocator-size 0x1200000000 \
#SF10:
#$jarvis_tools/mkgraph $graph --region-size 0x400000000 --allocator-size 0x800000000 --num-allocators 8 \
# ALL on a real PM system with enough to back the storage
$jarvis_tools/mkgraph $graph --num-allocators 8 \
    node Person id integer \
    node Tag name string \
    node Tag id integer \
    node Post id integer \
    node Comment id integer \
    node Language language string \
    node Forum id integer \
    node Place id integer

perl loadldbc.pl --graph $graph --loadgraph "$jarvis_tools/loadgraph" \
    --sep '|' --tag TagClass --tag EmailAddress --tag Language \
    $data/*_*.csv

