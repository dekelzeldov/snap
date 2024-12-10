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
        # print(f"Error: Failed to read {file_path}")
        return None


def make_graph_results(graph):
    # print(f"\tmake_graph_results")
    expected_results = {v: [] for v in Variants}
    found_results = {v: [] for v in Variants}

    overal_dict = get_dict(os.path.join(in_folder, graph, f"run_on_graph_{graph}.json"))

    for seed, seed_info_dict in overal_dict["Results"].items():

        cluseters = {}
        cluseters["Expected"] = set(seed_info_dict["Expected Cluster"])
        for variant in Variants:
            seed_results_dict = get_dict(seed_info_dict["Out Files"][variant])
            if not seed_results_dict:
                # print(f"missing result for:")
                print(f"{seed_info_dict['Run Commands'][variant]} > {seed_info_dict['Out Files'][variant]}")
                continue
            
            cluseters[variant] = set(seed_results_dict["Found Cluster"])
            expected_results[variant].append((seed_info_dict["Expected Cluster Size"], seed_results_dict["Run Time (seconds)"]))
            found_results[variant].append((seed_results_dict["Found Cluster Size"], seed_results_dict["Run Time (seconds)"]))

        if "Local" in cluseters.keys() and "Purely Local" in cluseters.keys():
            if cluseters["Local"] != cluseters["Purely Local"]:
                # print(f"Clusters are NOT the same for seed {seed}")
                # print(f"\t |Local-Purely|: {len(cluseters['Local']-cluseters['Purely Local'])}")
                # print(f"\t |Purely-Local|: {len(cluseters['Purely Local']-cluseters['Local'])}")
                # print(f"run commands:")
                # print(f"\t {seed_info_dict['Run Commands']['Local']} > {seed_info_dict['Out Files']['Local']}")
                print(f"{seed_info_dict['Run Commands']['Purely Local']} > {seed_info_dict['Out Files']['Purely Local']}")
        
    # print(f"\tcreating plots")
    for res, name in [(found_results, "Found")]: # (expected_results, "Ground Truth"), 
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

                
def make_multigraphs_results(graph_names):
    result_dicts = {}
    for graph in graph_names:
        totals = {}
        for variant in Variants:
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
            result_dicts[(graph, variant)] = {
                "Read Graph": f"{np.mean(read_runtimes)} \pm {np.std(read_runtimes)}",
                "Weight Computation": f"{np.mean(weight_runtimes)} \pm {np.std(weight_runtimes)}",
                "APPR": f"{np.mean(appr_runtimes)} \pm {np.std(appr_runtimes)}",
                "Rest": f"{np.mean(rest_runtimes)} \pm {np.std(rest_runtimes)}",
                "Total": f"{np.mean(total_runtimes)} \pm {np.std(total_runtimes)}",
                "speedup": "N/A"
            }
            totals[variant] = total_runtimes
        speedup = np.array(totals["Local"]) / np.array(totals["Purely Local"])
        result_dicts[(graph, "Purely Local")]["speedup"] = f"{np.mean(speedup)} \pm {np.std(speedup)}"
    df = pandas.DataFrame(result_dicts)
    with open(os.path.join(out_folder, "real_graphs_runtimes_tabel.tex"), 'w') as f:
        f.write(df.to_latex())

def make_speedup_results(dataset_list):
    result_dicts = {}
    for graph in dataset_list:
        overal_dict = get_dict(os.path.join(in_folder, graph["name"], f"run_on_graph_{graph['name']}.json"))
        speedups = []
        for _, seed_info_dict in overal_dict["Results"].items():
            p_seed_results_dict = get_dict(seed_info_dict["Out Files"]["Purely Local"])
            if not p_seed_results_dict:
                print(f"missing result for:")
                print(f"\t {seed_info_dict['Run Commands']['Purely Local']} > {seed_info_dict['Out Files']['Purely Local']}")
                continue
            l_seed_results_dict = get_dict(seed_info_dict["Out Files"]["Local"])
            if not p_seed_results_dict:
                print(f"missing result for:")
                print(f"\t {seed_info_dict['Run Commands']['Local']} > {seed_info_dict['Out Files']['Local']}")
                continue
            speedup = l_seed_results_dict["Run Time (seconds)"] / p_seed_results_dict["Run Time (seconds)"]
            speedups.append(speedup)
        if graph["-N"] not in result_dicts.keys():
            result_dicts[graph["-N"]] = {}
        result_dicts[graph["-N"]][graph["-mu"]] = f"{np.mean(speedups)} \pm {np.std(speedups)}"
    df = pandas.DataFrame(result_dicts)
    with open(os.path.join(out_folder, "synthetic_graphs_runtimes_speedup_tabel.tex"), 'w') as f:
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
