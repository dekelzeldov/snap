
import get_results_utils
import json


dataset_file = "./synthetic_data_datasets.json"
with open(dataset_file) as f:
    dataset_list = json.load(f)

for dataset in dataset_list:
    get_results_utils.check_volume(dataset["name"])

print(f"creating table")
get_results_utils.make_speedup_results(dataset_list)