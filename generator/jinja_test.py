# use the command "python .\jinja_test.py --jinja2-path D:\junwei\webgpu\dawn\third_party\jinja2"
import argparse, json, os, re, sys

kJinja2Path = '--jinja2-path'
try:
    jinja2_path_argv_index = sys.argv.index(kJinja2Path)
    # Add parent path for the import to succeed.
    path = os.path.join(sys.argv[jinja2_path_argv_index + 1], os.pardir)
    sys.path.insert(1, path)
except ValueError:
    # --jinja2-path isn't passed, ignore the exception and just import Jinja2
    # assuming it already is in the Python PATH.
    pass

from jinja2 import Environment, FileSystemLoader

persons = [
    {'name': 'Andrej', 'age': 18}, 
    {'name': 'Mark', 'age': 17}, 
    {'name': 'Thomas', 'age': 19},
]

file_loader = FileSystemLoader('templates')
env = Environment(loader=file_loader)

template = env.get_template('showpersons.txt')

output = template.render(persons=persons)
print(output)
