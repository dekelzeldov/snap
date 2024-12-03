import json
import os

folder = "./email-Eu-core"
for filename in os.listdir(folder):
    file = os.path.join(folder, filename)
    # checking if it is a file
    if os.path.isfile(file):
        with open(file) as f:
            d = json.load(f)
            print(d)