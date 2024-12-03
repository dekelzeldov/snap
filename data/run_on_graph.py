
import get_seeds
import os
import subprocess
import platform
import json

graph_file = "email-Eu-core.txt"

# Generate n random numbers as seeds
seed_data_file = "email-Eu-core-department-labels.txt"
num_seeds = 1

# Run paths
exe_paths = [
    "../examples/localmotifcluster/localmotifclustermain",
    "../examples/purelylocalmotifcluster/purelylocalmotifclustermain"
]

base_args = [
    "-i:"+graph_file,
    "-d:Y",
    "-m:FFLoop",
    "-silent:Y"
]

seeds = get_seeds.pick_seeds(seed_data_file, num_seeds)

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
    "Seeds": seeds,
    "Output Files": []
}

commands = []
for seed, _ in seeds:
    out_files = []
    for exe_path in exe_paths:
        # Set the log file name
        out_path = os.path.join(".", os.path.basename(graph_file).split(".")[0])
        out_file = os.path.join(out_path,f"{os.path.basename(exe_path)}_seed{seed}.log")
        if not os.path.exists(out_path):
            os.makedirs(out_path)
        # Run the command and capture the output
        command = [exe_path] + base_args + [f"-s:{seed}"]
        # command = ["srun"] + command
        print(f"Running: {' '.join(command)}")
        print(f"\tOutput File: {out_file}")
        commands.append(subprocess.Popen(command,
				stdout=open(out_file, 'w'),
				stderr=subprocess.STDOUT))
        out_files.append(out_file)
    run_info_dict["Output Files"].append(out_files)

with open(f'run_on_graph_{graph_file}.json', 'w') as fp:
    json.dump(run_info_dict, fp, separators=(',', ': '), indent=4)
    
for c in commands:
    c.wait()
    if c.returncode != 0:
        print(f"{c.args} \n\t exited with code: {c.returncode}")