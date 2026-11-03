"""Exercise the real presentation functions with a host-side mock GPU queue.

Run with Python and a host C++ compiler, for example:
    python tests/test_switch_saltynx_resolution.py --cxx g++
"""
import argparse
import os
from pathlib import Path
import subprocess
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument("--cxx", default=os.environ.get("CXX", "c++"))
args = parser.parse_args()
root = Path(__file__).resolve().parents[1]
source = (root / "platform/switch/deko/backend.cpp").read_text(encoding="utf-8")


def function(name):
    start = source.index("void " + name + "(")
    end = source.index("\n}", start) + 2
    return source[start:end] + "\n"


with tempfile.TemporaryDirectory(prefix="cane-saltynx-test-") as temporary:
    build = Path(temporary)
    (build / "saltynx_present.inc").write_text(
        function("SetFrameResolution") + function("Present"), encoding="utf-8")
    executable = build / ("resolution-test.exe" if os.name == "nt" else "resolution-test")
    subprocess.run([args.cxx, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-I" + str(build),
                    str(root / "tests/switch_saltynx_resolution.cpp"), "-o", str(executable)], check=True)
    subprocess.run([str(executable)], check=True)
print("ProjectCane: internal resolution, scaling, output fallback, docking and failed presents passed")
