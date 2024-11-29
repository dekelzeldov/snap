import random

def read_list_of_lists(file_path):
	with open(file_path, 'r') as file:
		list_of_lists = []
		for line in file:
			# Convert the line into a list of numbers
			number_list = list(map(int, line.strip().split()))
			list_of_lists.append(number_list)
	return list_of_lists

def get_k_longest_lists(lists, k):
	# Sort the lists by their length in descending order and get the first k lists
	return sorted(lists, key=len, reverse=True)[:k]

# Example usage
file_path = '/Users/dekel/Downloads/com-dblp.top5000.cmty.txt'
lists = read_list_of_lists(file_path)

k = 10  # Change this to the number of longest lists you want
longest_lists = get_k_longest_lists(lists, k)

chosen_nodes = []
for lst in longest_lists:
	# Choose a random node from each list
	chosen_nodes.append(random.choice(lst))
	
print(chosen_nodes)