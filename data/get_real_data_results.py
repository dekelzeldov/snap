
import get_results_utils
import json


dataset_file = "./real_data_datasets.json"
with open(dataset_file) as f:
    dataset_list = json.load(f)

for dataset in dataset_list:
    graph_name = dataset["name"]
    print(f"analizing {graph_name}")
    get_results_utils.check_volume(graph_name)
    get_results_utils.make_graph_results(graph_name)

print(f"creating table")
get_results_utils.make_multigraphs_results([graph["name"] for graph in dataset_list])