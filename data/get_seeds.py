import random

def read_list_of_lists(file_path):
	with open(file_path, 'r') as file:
		list_of_lists = []
		for line in file:
			# Convert the line into a list of numbers
			number_list = list(map(int, line.strip().split()))
			list_of_lists.append(number_list)
	return list_of_lists

def read_labels(file_path):
	with open(file_path, 'r') as file:
		labels = {}
		for line in file:
			node, label = map(int, line.strip().split())
			if label not in labels:
				labels[label] = []
			labels[label].append(node)
	return labels.values()

def get_k_longest_lists(lists, k):
	# Sort the lists by their length in descending order and get the first k lists
	return sorted(lists, key=len, reverse=True)[:k]

# Example usage
def pick_seeds(file, k):
	# lists = read_list_of_lists(file)
	lists = read_labels(file)

	longest_lists = get_k_longest_lists(lists, k)

	chosen_nodes = []
	for lst in longest_lists:
		# Choose a random node from each list
		chosen_nodes.append((random.choice(lst), len(lst)))
		
	return chosen_nodes