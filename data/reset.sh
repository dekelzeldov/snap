cd ../examples/localmotifcluster/ 
make 2>&1 >/dev/null | grep -i "error"
cd ../purelylocalmotifcluster/
make 2>&1 >/dev/null | grep -i "error"
cd ../../data/
rm -rf ./email-Eu-core/*.log
python3 run_on_graph.py
python3 get_results.py 