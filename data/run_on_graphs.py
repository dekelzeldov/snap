
import get_seeds
import os
import subprocess
import platform
import json
import get_results_utils

run=0
# Generate n random numbers as seeds
num_seeds = 100
rand_seed = None

just_get_volume = False
syn_or_real = "real"

if syn_or_real == "real":
    dataset_file = f"{os.getcwd()}/real_data/real_data_datasets.json"
    with open(dataset_file) as f:
        dataset_list = json.load(f)
if syn_or_real == "syn":
    dataset_file = f"{os.getcwd()}/synthetic_data/synthetic_data_datasets.json"
    with open(dataset_file) as f:
        dataset_list = json.load(f)
    
# Run paths
exe_paths = {
    "Local": [f"{os.getcwd()}/../examples/localmotifcluster/localmotifclustermain"],
    "Purely Local": [f"{os.getcwd()}/../examples/purelylocalmotifcluster/purelylocalmotifclustermain"],
}

if just_get_volume:
    exe_paths.pop("Purely Local")

system_info = {
    "System": platform.system(),
    "Release": platform.release(),
    "Version": platform.version(),
    "Machine": platform.machine(),
    "Processor": platform.processor(),
    "Architecture": platform.architecture()
}

for dataset in dataset_list:
    if syn_or_real == "syn":
        graph_name = dataset["name"]
        graph_file = os.path.join(dataset["folder"], "network.dat")
        seed_data_file = os.path.join(dataset["folder"], "community.dat")
        labels_or_lists = "labels"
    if syn_or_real == "real":
        graph_name = dataset["name"]
        graph_file = dataset["graph_path"]
        seed_data_file = dataset["seed_data_path"]
        labels_or_lists = dataset["labels_or_lists"]
    print(f"running on {graph_name}")
    motif = dataset["motif"]
    base_args = []
    base_args.append(f"-i:{os.getcwd()+'/'+graph_file}")
    base_args.append(f"-d:{'Y' if dataset['directed'] else 'N'}")
    base_args.append(f"-m:{motif}")
    base_args.append(f"-silent:Y")
    if just_get_volume:
        base_args.append("-v:Y")

    seeds = get_seeds.pick_seeds(seed_data_file, labels_or_lists, num_seeds, seed=rand_seed)

    out_path = os.path.join(os.getcwd(), "results" ,graph_name)
    out_files_path = os.path.join(out_path, "seeds_results")
    run_on_graph_file = os.path.join(out_path,f'run_on_graph_{graph_name}.json')

    Total_Volume = get_results_utils.get_total_volume(graph_name, motif)

    if just_get_volume and Total_Volume:
        continue

    run_info_dict = {
        "System Info": system_info,
        "Graph File":  graph_file,
        "Exicutions": exe_paths,
        "Directed": dataset['directed'],
        "Motif": motif,
        "Results": {}
    }

    if just_get_volume:
        run_info_dict["Just For Volume"] = True

    commands = []
    for seed, expected in seeds:
        print(f"\t seed {seed}")
        result = {}
        result["Expected Cluster Size"] = len(expected)
        result["Expected Cluster"] = expected
        result["Run Commands"] = {}
        result["Out Files"] = {}
        for variant, exe_path in exe_paths.items():
            # Set the log file name
            out_file = os.path.join(out_files_path, f"{variant.lower().replace(' ','_')}_seed{seed}_run{run}.json")
            err_file = os.path.join(out_files_path, f"{variant.lower().replace(' ','_')}_seed{seed}_run{run}.err")
            if not os.path.exists(out_files_path):
                os.makedirs(out_files_path)

            # Run the command and capture the output
            command = exe_path + base_args + [f"-s:{seed}"]
            if variant == "Purely Local" and Total_Volume:
                command.append(f"-v:{Total_Volume}")
            command = ["srun"] + command 

            result["Run Commands"][variant] = " ".join(command)
            result["Out Files"][variant] = out_file

            commands.append(subprocess.Popen(command,
                    stdout=open(out_file, 'w'),
                    stderr=open(err_file, 'w')))


        run_info_dict["Results"][seed] = result

        if just_get_volume:
            break

    with open(run_on_graph_file, 'w') as fp:
        json.dump(run_info_dict, fp, separators=(',', ': '), indent=4)



