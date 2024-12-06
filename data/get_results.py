import json
import os
from matplotlib import pyplot as plt
import numpy as np

Variants = ["Purely Local", "Local"]

def get_results(graph):
    expected_results = {v: [] for v in Variants}
    found_results = {v: [] for v in Variants}

    folder = os.path.join(".", graph)

    Total_Volume = None
    with open(os.path.join(folder, f"run_on_graph_{graph}.json")) as f:
        overal_dict = json.load(f)
        if "Total Volume" in overal_dict.keys():
            Total_Volume = overal_dict["Total Volume"]
        for seed, seed_info_dict in overal_dict["Results"].items():

            cluseters = {}
            cluseters["Expected"] = set(seed_info_dict["Expected Cluster"])
            for variant in Variants:
                with open(seed_info_dict[variant]) as f:
                    seed_results_dict = json.load(f)

                    if "Total Volume" in seed_results_dict.keys():
                        if Total_Volume is None:
                            Total_Volume = seed_results_dict["Total Volume"]
                        else:
                            assert Total_Volume == seed_results_dict["Total Volume"]
                    
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
            plt.scatter(xs, ys)  
        plt.xlabel(f"{name} Cluster Size")
        plt.ylabel("Run Time (seconds)")
        plt.title(f"Run Time vs {name} Cluster Size")
        plt.legend(list(Variants)) 
        plt.savefig(os.path.join(folder, f"{variant}_run_time_vs_{name.lower().replace(' ', '_')}_cluster_size.png"))


def get_volume(graph):
    folder = os.path.join(".", graph)
    Total_Volume = None
    with open(os.path.join(folder, f"run_on_graph_{graph}.json")) as f:
        overal_dict = json.load(f)
        if "Total Volume" in overal_dict.keys():
            Total_Volume = overal_dict["Total Volume"]
        for _, seed_info_dict in overal_dict["Results"].items():
            for variant in Variants:
                with open(seed_info_dict[variant]) as f:
                    seed_results_dict = json.load(f)

                    if "Total Volume" in seed_results_dict.keys():
                        if Total_Volume is None:
                            Total_Volume = seed_results_dict["Total Volume"]
                        else:
                            assert Total_Volume == seed_results_dict["Total Volume"]
    return Total_Volume