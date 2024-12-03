
import get_seeds
import get_results
import os
import subprocess
import platform
import json

graph_file = "com-dblp.ungraph.txt"

# Generate n random numbers as seeds
seed_data_file = "com-dblp.top5000.cmty.txt"
labels_or_lists = "lists"
num_seeds = 10

# Run paths
exe_paths = {
    "Local": "../examples/localmotifcluster/localmotifclustermain",
    "Purely Local": "../examples/purelylocalmotifcluster/purelylocalmotifclustermain",
}

base_args = [
    "-i:"+graph_file,
    "-d:N",
    "-m:clique3",
    "-silent:Y"
]

seeds = get_seeds.pick_seeds(seed_data_file, labels_or_lists, num_seeds, seed=None)

def system_info():
    system_info = {
        "System": platform.system(),
        "Release": platform.release(),
        "Version": platform.version(),
        "Machine": platform.machine(),
        "Processor": platform.processor(),
        "Architecture": platform.architecture()
    }
    return system_info

run_info_dict = {
    "System Info": system_info(),
    "Graph File":  graph_file,
    "Exicutions": exe_paths,
    "Base Arguments": base_args,
    "Results": {}
}

graph_file_name = ".".join(os.path.basename(graph_file).split(".")[:-1])
out_path = os.path.join(".",graph_file_name)
out_files_path = os.path.join(out_path, "seeds_results")
commands = []
for seed, expected in seeds:
    result = {}
    result["Expected Cluster Size"] = len(expected)
    result["Expected Cluster"] = expected
    for variant, exe_path in exe_paths.items():
        # Set the log file name
        out_file = os.path.join(out_files_path, f"{os.path.basename(exe_path)}_seed{seed}.log")
        if not os.path.exists(out_files_path):
            os.makedirs(out_files_path)

        # Run the command and capture the output
        command = [exe_path] + base_args + [f"-s:{seed}"]
        # command = ["srun"] + command

        # print(f"Running: {' '.join(command)}")
        # print(f"\tOutput File: {out_file}")

        commands.append(subprocess.Popen(command,
				stdout=open(out_file, 'w'),
				stderr=subprocess.STDOUT))
        result[variant] = out_file

    run_info_dict["Results"][seed] = result

with open(os.path.join(out_path,f'run_on_graph_{graph_file_name}.json'), 'w') as fp:
    json.dump(run_info_dict, fp, separators=(',', ': '), indent=4)
    
for c in commands:
    c.wait()
    if c.returncode != 0:
        print(f"{c.args} \n\t exited with code: {c.returncode}")
get_results.get_results(graph_file_name)