
import json
import os
import subprocess

target_folder = "./synthetic_data/"
relative_script_path = "../../../../LFRbenchmarks/unweighted_undirected/benchmark"
os.makedirs(target_folder, exist_ok=True)

graphs = {}

for num, n in [('10k', 10000)]: #, ('100k', 100000), ('1m', 1000000)]:
	for mu in [0.1, 0.5, 0.9]:
		graph_name = f"network_{num}_mu{mu}"
		graph_folder = os.path.join(target_folder, graph_name)
		os.makedirs(graph_folder, exist_ok=True)
		args_dict = {
			"-N": n,
			"-k": 20,
			"-maxk": 200,
			"-mu": mu
		}
		base_args = " ".join([f"{k} {v}" for k, v in args_dict.items()])
		cmd = [relative_script_path] + base_args.split()
		graphs[graph_name] = args_dict
		graphs[graph_name]["directed"] = False
		graphs[graph_name]["motif"] = "clique3"
		graphs[graph_name]["folder"] = graph_folder
		graphs[graph_name]["cmd"] = cmd
		proc = subprocess.Popen(cmd, cwd=graph_folder, stdout=open(os.path.join(graph_folder , "output.txt"), "w"))
		proc.wait()
		if proc.returncode != 0:
			print(f"graph_folder: {graph_folder}")
			print(f"{' '.join(proc.args)} \n\t exited with code: {proc.returncode}")

with open(os.path.join(".", "synthtic_data_datasets.json"), "w") as f:
    json.dump(graphs, f, separators=(',', ':'), indent=4)