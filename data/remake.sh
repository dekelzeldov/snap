cd ../examples/localmotifcluster/ 
make 2>&1 >/dev/null | grep -i "error"
cd ../purelylocalmotifcluster/
make 2>&1 >/dev/null | grep -i "error"
cd ../../data/