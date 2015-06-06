
import os
from random import randint, sample
import string
base_dir = "source"


char_set1 = list(string.ascii_lowercase+string.ascii_uppercase)

def random_name(length):
    return ''.join(sample(char_set1, length))

def random_fileset(number):
    return [random_name(randint(4,5)) for i in range(number)]

def ensure_directory(directory):
    if not os.path.exists(directory):
        os.makedirs(directory)
    else:
        pass

def contruct_tree(depth):
    # TODO Needs a recursive form
    # so we can handle depth
    root = {}
    tree = {}
    for dirname in random_fileset(2):
        child = {}
        for d2 in random_fileset(5):
            child2 = {}
            for d3 in random_fileset(8):
                child2[d3] = {}
            child[d2] = child2
        tree[dirname] = child
    root["source"] = tree
    return(root)


def generate_header():
    header = """
/****f* {}/{}
 * NAME
 *    {}
 * FUNCTION
 *    vla flip too
 *    horton hears a who
 *****/

"""
    name = random_name(10)
    header = header.format(name, random_name(10), name)
    return header

def generate_source_tree(tree, path):
    for filename in tree:
        subtree = tree[filename]
        if len(subtree) == 0:
            ensure_directory(path)
            source_name = filename + ".c"
            source_path = os.path.join(path, source_name)
            print(source_path)
            with open(source_path, "w") as oupf:
                header = generate_header()
                oupf.write(header)
        else:
            generate_source_tree(
                    tree[filename], os.path.join(path,filename))

tree = contruct_tree(4)
print(tree)
generate_source_tree(tree, "")



