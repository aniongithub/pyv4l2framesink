import subprocess
from setuptools import setup, find_packages

subprocess.run(["cmake", "."])
subprocess.run("make")

setup(
    name = 'pyv4l2framesink',
    version = 0.1,
    packages = find_packages(where = "./pyv4l2framesink")
)