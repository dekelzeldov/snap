
import get_seeds
import get_results
import os
import subprocess
import platform
import json

run=0
# Generate n random numbers as seeds
num_seeds = 10
rand_seed = None

graph_file = "com-dblp.ungraph.txt"
seed_data_file = "com-dblp.top5000.cmty.txt"
labels_or_lists = "lists"

base_args = [
    "-i:"+graph_file,
    "-d:N",
    "-m:clique3",
    "-silent:Y"
]

# graph_file = "email-Eu-core.txt"
# seed_data_file = "email-Eu-core-department-labels.txt"
# labels_or_lists = "labels"

# base_args = [
#     "-i:"+graph_file,
#     "-d:Y",
#     "-m:FFLoop",
#     "-silent:Y"
# ]

# graph_file = "wiki-topcats.txt"
# seed_data_file = "wiki-topcats-categories.txt"
# labels_or_lists = "lists"

# base_args = [
#     "-i:"+graph_file,
#     "-d:Y",
#     "-m:FFLoop",
#     "-silent:Y"
# ]


# Run paths
exe_paths = {
    "Local": ["../examples/localmotifcluster/localmotifclustermain"],
    "Purely Local": ["../examples/purelylocalmotifcluster/purelylocalmotifclustermain"],
}

seeds = get_seeds.pick_seeds(seed_data_file, labels_or_lists, num_seeds, seed=rand_seed)

graph_file_name = ".".join(os.path.basename(graph_file).split(".")[:-1])
out_path = os.path.join(".",graph_file_name)
out_files_path = os.path.join(out_path, "seeds_results")
run_on_graph_file = os.path.join(out_path,f'run_on_graph_{graph_file_name}.json')

if os.path.isfile(run_on_graph_file):
    with open(run_on_graph_file) as f:
        past_run_info_dict = json.load(f)
        if "Total Volume" in past_run_info_dict.keys():
            exe_paths["Purely Local"].append(f"-v:{past_run_info_dict['Total Volume']}")

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

commands = []
for seed, expected in seeds:
    result = {}
    result["Expected Cluster Size"] = len(expected)
    result["Expected Cluster"] = expected
    for variant, exe_path in exe_paths.items():
        # Set the log file name
        out_file = os.path.join(out_files_path, f"{os.path.basename('_'.join(exe_path))}_seed{seed}_run{run}.json")
        if not os.path.exists(out_files_path):
            os.makedirs(out_files_path)

        # Run the command and capture the output
        command = exe_path + base_args + [f"-s:{seed}"]
        # command = ["srun"] + command

        print(f"Running: {' '.join(command)}")
        print(f"\tOutput File: {out_file}")

        commands.append(subprocess.Popen(command,
				stdout=open(out_file, 'w'),
				stderr=subprocess.STDOUT))
        result[variant] = out_file

    run_info_dict["Results"][seed] = result


with open(run_on_graph_file, 'w') as fp:
    json.dump(run_info_dict, fp, separators=(',', ': '), indent=4)

for c in commands:
    c.wait()
    if c.returncode != 0:
        print(f"{' '.join(c.args)} \n\t exited with code: {c.returncode}")
get_results.get_results(graph_file_name)

run_info_dict["Total Volume"] = get_results.get_volume(graph_file_name)
with open(run_on_graph_file, 'w') as fp:
    json.dump(run_info_dict, fp, separators=(',', ': '), indent=4)
