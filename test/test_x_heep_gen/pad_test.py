import json
from pathlib import Path
import os
from deepdiff import DeepDiff


def compare_json(file_a, file_b):
    with open(os.path.join(os.path.dirname(__file__), file_a)) as fa, open(
        os.path.join(os.path.dirname(__file__), file_b)
    ) as fb:
        a = json.load(fa)
        b = json.load(fb)

    if a == b:
        print("✅ JSONs are identical")
        return True
    else:
        print("❌ JSONs differ")
        diff = DeepDiff(a, b, ignore_order=True, significant_digits=6)
        print(diff)
        return False


if __name__ == "__main__":
    compare_json(
        "./pads/golden_pads/kwargs_output.json", "./pads/output/kwargs_output.json"
    )
