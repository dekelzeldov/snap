import json
import os

folder = "./email-Eu-core"
for filename in os.listdir(folder):
    file = os.path.join(folder, filename)
    # checking if it is a file
    if os.path.isfile(file):
        with open(file) as f:
            # remove newlines from the file
            f = ' '.join(line.strip() for line in f)
            print(f)
            d = json.loads(f)
            for key, value in d.items():
                print("Element:")
                print(f"key: {key}")
                print(f"value: \n{value}")
