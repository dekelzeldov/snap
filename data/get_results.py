import json
import os
from matplotlib import pyplot as plt
import numpy as np

def get_results(graph):
    Variants = ["Purely Local", "Local"]
    expected_results = {v: [] for v in Variants}
    found_results = {v: [] for v in Variants}

    folder = os.path.join(".", graph)

    with open(os.path.join(folder, f"run_on_graph_{graph}.json")) as f:
        overal_dict = json.load(f)
        for _, seed_info_dict in overal_dict["Results"].items():
            cluseters = {}
            cluseters["Expected"] = set(seed_info_dict["Expected Cluster"])
            for variant in Variants:
                with open(seed_info_dict[variant]) as f:
                    seed_results_dict = json.load(f)
                    
                    cluseters[variant] = set(seed_results_dict["Found Cluster"])
                    expected_results[variant].append((seed_info_dict["Expected Cluster Size"], seed_results_dict["Run Time (seconds)"]))
                    found_results[variant].append((seed_results_dict["Found Cluster Size"], seed_results_dict["Run Time (seconds)"]))

            if cluseters["Local"] != cluseters["Purely Local"]:
                print("Clusters are not the same")
                print(f"\t Local-Purely: {cluseters['Local']-cluseters['Purely Local']}")
                print(f"\t Purely-Local: {cluseters['Purely Local']-cluseters['Local']}")
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
        plt.savefig(os.path.join(folder, f"{variant}_run_time_vs_{name.lower().replace(" ", "_")}_cluster_size.png"))

