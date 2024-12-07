import json
import os
from matplotlib import pyplot as plt
import numpy as np

Variants = ["Purely Local", "Local"]

def make_graph_results(graph):
    expected_results = {v: [] for v in Variants}
    found_results = {v: [] for v in Variants}

    folder = os.path.join(".", "results", graph)

    with open(os.path.join(folder, f"run_on_graph_{graph}.json")) as f:
        overal_dict = json.load(f)

        for seed, seed_info_dict in overal_dict["Results"].items():

            cluseters = {}
            cluseters["Expected"] = set(seed_info_dict["Expected Cluster"])
            for variant in Variants:
                with open(seed_info_dict[variant]) as f:
                    seed_results_dict = json.load(f)
                    
                    cluseters[variant] = set(seed_results_dict["Found Cluster"])
                    expected_results[variant].append((seed_info_dict["Expected Cluster Size"], seed_results_dict["Run Time (seconds)"]))
                    found_results[variant].append((seed_results_dict["Found Cluster Size"], seed_results_dict["Run Time (seconds)"]))

            if cluseters["Local"] != cluseters["Purely Local"]:
                print(f"Clusters are NOT the same for seed {seed}")
                print(f"\t |Local-Purely|: {len(cluseters['Local']-cluseters['Purely Local'])}")
                print(f"\t |Purely-Local|: {len(cluseters['Purely Local']-cluseters['Local'])}")
            else:
                print("Clusters are the same")

    for res, name in [(expected_results, "Ground Truth"), (found_results, "Found")]:
        plt.figure()
        for variant in Variants:
            xs, ys = zip(*res[variant])
            xs, ys = np.array(list(xs)), np.array(list(ys))
            plt.scatter(xs, ys, alpha=0.5)  
        plt.xlabel(f"{name} Cluster Size")
        plt.ylabel("Run Time (seconds)")
        plt.ylim(0, 1.1*max(ys))
        plt.title(f"Run Time vs {name} Cluster Size")
        plt.legend(list(Variants)) 
        plt.savefig(os.path.join(folder, f"run_time_vs_{name.lower().replace(' ', '_')}_cluster_size.png"))

def get_accumilated_graph_results(graph, variant):
    folder = os.path.join(".", "results", graph)
    read_runtimes = []
    weight_runtimes = []
    appr_runtimes = []
    rest_runtimes = []
    total_runtimes = []
    with open(os.path.join(folder, f"run_on_graph_{graph}.json")) as f:
        overal_dict = json.load(f)
        for _, seed_info_dict in overal_dict["Results"].items():
            with open(seed_info_dict[variant]) as f:
                seed_results_dict = json.load(f)
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
    result_dicts = []
    for graph in graph_names:
        for variant in Variants:
            result_dicts.append(get_accumilated_graph_results(graph, variant))

    fig, ax = plt.subplots()

    # ax.plot(Variants*len(graph_names), [d["Read Graph"][0] for d in result_dicts], 'o')
    ax.errorbar(Variants*len(graph_names), [d["Read Graph"][0] for d in result_dicts], yerr=[d["Read Graph"][1] for d in result_dicts],ecolor='black',marker='o',ls='')

    # ax.plot(Variants*len(graph_names), [d["Weight Computation"][0] for d in result_dicts], 'o')
    ax.errorbar(Variants*len(graph_names), [d["Weight Computation"][0] for d in result_dicts], yerr=[d["Weight Computation"][1] for d in result_dicts],ecolor='black',marker='o',ls='')

    # ax.plot(Variants*len(graph_names), [d["APPR"][0] for d in result_dicts], 'o')
    ax.errorbar(Variants*len(graph_names), [d["APPR"][0] for d in result_dicts], yerr=[d["APPR"][1] for d in result_dicts],ecolor='black',marker='o',ls='')

    # ax.plot(Variants*len(graph_names), [d["Rest"][0] for d in result_dicts], 'o')
    ax.errorbar(Variants*len(graph_names), [d["Rest"][0] for d in result_dicts], yerr=[d["Rest"][1] for d in result_dicts],ecolor='black',marker='o',ls='')

    # ax.plot(Variants*len(graph_names), [d["Total"][0] for d in result_dicts], 'o')
    ax.errorbar(Variants*len(graph_names), [d["Total"][0] for d in result_dicts], yerr=[d["Total"][1] for d in result_dicts],ecolor='black',marker='o',ls='')

    # label the graphs:
    sec = ax.secondary_xaxis(location=0)

    sec.set_xticks([0.5+2*x for x in range(len(graph_names))], labels=graph_names)
    sec.tick_params('x', length=0)

    # lines between the classes:
    sec2 = ax.secondary_xaxis(location=0)
    sec2.set_xticks([-0.5+2*x for x in range(len(graph_names)+1)], labels=[])
    sec2.tick_params('x', length=40, width=1.5)
    sec2.set_label("Graphs and Tool")
    ax.set_xlim(-0.6, 8.6)

    ax.set_ylabel("Time (seconds)")
    ax.set_title("Run Time per Variant per Graph")
    ax.legend(["Read Graph", "Weight Computation", "APPR", "Rest", "Total"])
    plt.savefig(os.path.join(".", "results", graph, "run_time_per_variant_per_graph.png"))

def get_total_volume(graph, motif):
    folder = os.path.join(".", "results", graph)
    volume_json = os.path.join(folder, f"total_volumes_{graph}.json")
    if os.path.isfile(volume_json):
        with open(volume_json) as f:
            volume_dict = json.load(f)
            if motif in volume_dict.keys():
                return volume_dict[motif]
    return None

def set_total_volume(graph, motif, volume):
    folder = os.path.join(".", "results", graph)
    volume_json = os.path.join(folder, f"total_volumes_{graph}.json")
    volume_dict = {}
    if os.path.isfile(volume_json):
        with open(volume_json) as f:
            volume_dict = json.load(f)
    volume_dict[motif] = volume
    with open(volume_json, "w") as f:
        json.dump(volume_dict, f)

def check_volume(graph):
    folder = os.path.join(".", "results", graph)
    graph_json = os.path.join(folder, f"run_on_graph_{graph}.json")
    with open(graph_json) as f:
        overal_dict = json.load(f)
        Total_Volume = get_total_volume(graph, overal_dict["Motif"])
        for _, seed_info_dict in overal_dict["Results"].items():
            for variant in Variants:
                if variant in seed_info_dict.keys():
                    with open(seed_info_dict[variant]) as f:
                        seed_results_dict = json.load(f)
                        if "Total Volume" in seed_results_dict.keys():
                            if Total_Volume is None:
                                Total_Volume = seed_results_dict["Total Volume"]
                            else:
                                assert Total_Volume == seed_results_dict["Total Volume"], f"Total Volume is not as expected in {graph_json} or not consistent between runs"
        set_total_volume(graph, overal_dict["Motif"], Total_Volume)