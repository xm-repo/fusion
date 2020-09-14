import json
import random

if __name__ == "__main__":

    record = '{"ts_fact" : 946684800, "fact_name": "fact", "actor_id": 111222, \
        "props": {"prop1": 11, "prop2": 22, "prop3": 24, "prop10": 1010}}'

    record = json.loads(record)

    for j in range(1, 20):        
        with open(f"file{j}.log", "w") as f:

            for i in range(100000):

                record["ts_fact"] += random.randint(0, 30)
                record["fact_name"] = "fact" + str(random.randint(0, 10))
                
                for k in range(1, 11):
                    record["props"][f"prop{k}"] = random.randint(0, 1)

                f.write(json.dumps(record) + "\n")
