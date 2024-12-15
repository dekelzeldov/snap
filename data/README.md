This is the enviorment for comparing the purely local motif cluster (L-MAPPR) to the local motif cluster (MAPPR) algorithem.

The L-MAPPR algorithem was developed by copying the MAPPR files:
	../snap-adv/localmotifcluster.h
	../snap-adv/localmotifcluster.cpp
to
	../snap-adv/purelylocalmotifcluster.h
	../snap-adv/purelylocalmotifcluster.cpp
and modifing them.

The algorithems can be run using:
	../examples/purelylocalmotifcluster/localmotifclustermain.cpp
and 
	../examples/purelylocalmotifcluster/purelylocalmotifclustermain.cpp
corespondingly.

To set up the inviorment run:
	./remake.sh

To run the real-data expiraments:
1. the real world data needs to be downloded and placed in the ./real_data folder
2. the ./real_data/real_data_datasets.json file needs to be updated acordingly
3. When running a network for the firs time:
	3.1 In the file ./run_on_graphs.py set the "just_get_volume" variable to true
	3.2 In the file ./get_real_data_results.py set the "just_get_volume" variable to true
	run:
		python3 run_on_graphs.py
		python3 get_real_data_results.py
	3.3 set both "just_get_volume" variable back to false
4. In the file ./run_on_graphs.py set:
	4.1 the variable "num_seeds" to the number of seed to run per graph
	4.2 the variable syn_or_real to "real"
5. run:
	python3 run_on_graphs.py
	python3 get_real_data_results.py

To run the synthtic expiraments:
1. generate the synthetic data:
	1.1 download and install the LFR benchmarks in the parent directory of this snap repository: https://github.com/andrealancichinetti/LFRbenchmarks 
	1.2 set the wanted variables in the create_synthetic.py script in the args_dict variable
	1.3 run:
		python3 create_synthetic.py
2. the ./synthetic_data/synthetic_data_datasets.json file will to be updated automaticly by the script
3. When running a network for the firs time:
	3.1 In the file ./run_on_graphs.py set the "just_get_volume" variable to true
	3.2 In the file ./get_synthetic_data_results.py set the "just_get_volume" variable to true
	run:
		python3 run_on_graphs.py
		python3 get_real_data_results.py
	3.3 set both "just_get_volume" variable back to false
4. In the file ./run_on_graphs.py set:
	4.1 the variable "num_seeds" to the number of seed to run per graph
	4.2 the variable syn_or_real to "real"
5. run:
	python3 run_on_graphs.py
	python3 get_syn_data_results.py

All results (figures and latex tabels) can be found in ./results/figures/