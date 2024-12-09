import json
import os
from matplotlib import pyplot as plt
import numpy as np
import pandas

Variants = ["Purely Local", "Local"]

in_folder = os.path.join(".", "results")
out_folder = os.path.join(in_folder, "figures")

def get_dict(file_path):
    try:
        with open(file_path) as f:
            content = f.read()
            first = content.find('{')
            return json.loads(content[first:])
    except:
        print(f"Error: Failed to read {file_path}")
        return None


def make_graph_results(graph):
    expected_results = {v: [] for v in Variants}
    found_results = {v: [] for v in Variants}

    overal_dict = get_dict(os.path.join(in_folder, graph, f"run_on_graph_{graph}.json"))

    for seed, seed_info_dict in overal_dict["Results"].items():

        cluseters = {}
        cluseters["Expected"] = set(seed_info_dict["Expected Cluster"])
        for variant in Variants:
            seed_results_dict = get_dict(seed_info_dict["Out Files"][variant])
            if not seed_results_dict:
                print(f"missing result for:")
                print(f"{seed_info_dict['Run Commands'][variant]} > {seed_info_dict['Out Files'][variant]}")
                continue
            
            cluseters[variant] = set(seed_results_dict["Found Cluster"])
            expected_results[variant].append((seed_info_dict["Expected Cluster Size"], seed_results_dict["Run Time (seconds)"]))
            found_results[variant].append((seed_results_dict["Found Cluster Size"], seed_results_dict["Run Time (seconds)"]))

        try:
            if cluseters["Local"] != cluseters["Purely Local"]:
                print(f"Clusters are NOT the same for seed {seed}")
                print(f"\t |Local-Purely|: {len(cluseters['Local']-cluseters['Purely Local'])}")
                print(f"\t |Purely-Local|: {len(cluseters['Purely Local']-cluseters['Local'])}")
                print(f"run commands:")
                print(f"\t {seed_info_dict['Run Commands']['Local']} > {seed_info_dict['Out Files']['Local']}")
                print(f"\t {seed_info_dict['Run Commands']['Purely Local']} > {seed_info_dict['Out Files']['Purely Local']}")
        except:
            print(f"\tcould not compare culsters for seed {seed} in graph {graph}")

    for res, name in [(expected_results, "Ground Truth"), (found_results, "Found")]:
        for log in [True, False]:
            plt.figure()
            min_y = 0.5
            max_y = 1
            for variant in Variants:
                xs, ys = zip(*res[variant])
                xs, ys = np.array(list(xs)), np.array(list(ys))
                plt.scatter(xs, ys, alpha=0.5)
                min_y = min(min(ys)/2, min_y)
                max_y = max(max(ys)*2, max_y)
            plt.xlabel(f"{name} Cluster Size")
            plt.ylabel("Run Time (seconds)")
            if log:
                plt.xscale("log")
                plt.yscale("log")
                plt.ylim(min_y, max_y)
            plt.title(f"Run Time vs {name} Cluster Size for {graph}")
            plt.legend(list(Variants), loc = 'lower right') 
            plt.savefig(os.path.join(out_folder, f"run_time_vs_{name.lower().replace(' ', '_')}_cluster_size{'_log' if log else ''}_{graph}.png"))

def get_accumilated_graph_results(graph, variant):
    read_runtimes = []
    weight_runtimes = []
    appr_runtimes = []
    rest_runtimes = []
    total_runtimes = []
    overal_dict = get_dict(os.path.join(in_folder, graph, f"run_on_graph_{graph}.json"))
    for _, seed_info_dict in overal_dict["Results"].items():
        seed_results_dict = get_dict(seed_info_dict["Out Files"][variant])
        if not seed_results_dict:
            continue
        read_runtimes.append(seed_results_dict["Read Graph Time (seconds)"])
        weight_runtimes.append(seed_results_dict["Weight Computation Time (seconds)"])
        appr_runtimes.append(seed_results_dict["APPR Time (seconds)"])
        total_runtimes.append(seed_results_dict["Run Time (seconds)"])
        rest_runtimes.append(total_runtimes[-1] - sum([read_runtimes[-1], weight_runtimes[-1], appr_runtimes[-1]]))
    return {
        "Read Graph": (np.mean(read_runtimes), np.std(read_runtimes)),
        "Weight Computation": (np.mean(weight_runtimes), np.std(weight_runtimes)),
        "APPR": (np.mean(appr_runtimes), np.std(appr_runtimes)),
        "Rest": (np.mean(rest_runtimes), np.std(rest_runtimes)),
        "Total": (np.mean(total_runtimes), np.std(total_runtimes))
    }

                
def make_multigraphs_results(graph_names):
    result_dicts = {}
    for graph in graph_names:
        for variant in Variants:
            result_dicts[(graph, variant)] = get_accumilated_graph_results(graph, variant)
    df = pandas.DataFrame(result_dicts)
    with open(os.path.join(out_folder, "real_graphs_runtimes_tabel.tex"), 'w') as f:
        f.write(df.to_latex())


def get_total_volume(graph, motif):
    volume_json = os.path.join(in_folder, graph, f"total_volumes_{graph}.json")
    if os.path.isfile(volume_json):
        volume_dict = get_dict(volume_json)
        if motif in volume_dict.keys():
            return volume_dict[motif]
    return None

def set_total_volume(graph, motif, volume):
    volume_json = os.path.join(in_folder, graph, f"total_volumes_{graph}.json")
    volume_dict = {}
    if os.path.isfile(volume_json):
        volume_dict = get_dict(volume_json)
    volume_dict[motif] = volume
    with open(volume_json, "w") as f:
        json.dump(volume_dict, f)

def check_volume(graph):
    graph_json = os.path.join(in_folder, graph, f"run_on_graph_{graph}.json")
    save_later = False
    overal_dict = get_dict(graph_json)
    Total_Volume = get_total_volume(graph, overal_dict["Motif"])
    if not Total_Volume:
        save_later = True
    for _, seed_info_dict in overal_dict["Results"].items():
        for variant in seed_info_dict['Out Files'].keys():
            seed_results_dict = get_dict(seed_info_dict['Out Files'][variant])
            if not seed_results_dict:
                continue
            if "Total Volume" in seed_results_dict.keys():
                if Total_Volume is None:
                    Total_Volume = seed_results_dict["Total Volume"]
                else:
                    assert Total_Volume == seed_results_dict["Total Volume"], f"Total Volume is not as expected in {graph_json} or not consistent between runs"
    if save_later:
        set_total_volume(graph, overal_dict["Motif"], Total_Volume)
