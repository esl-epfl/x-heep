import json
from pathlib import Path
import os

def compare_json(file_a, file_b):
    with open(os.path.join(os.path.dirname(__file__),file_a)) as fa, open(os.path.join(os.path.dirname(__file__),file_b)) as fb:
        a = json.load(fa)
        b = json.load(fb)

    if a == b:
        print("✅ JSONs are identical")
        return True
    else:
        print("❌ JSONs differ")
        # Optional: print details
        from deepdiff import DeepDiff
        diff = DeepDiff(a, b, ignore_order=True, significant_digits=6)
        print(diff)
        return False

if __name__ == "__main__":
    compare_json("../../util/golden_jsons/pads.json", "../../util/generated_jsons/pads.json")
