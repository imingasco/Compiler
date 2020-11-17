import os
import sys
from argparse import ArgumentParser

import graphviz

def main():
    parser = ArgumentParser()
    parser.add_argument('input_file', help='filename of input file to be visualized')
    args = parser.parse_args()
    input_file = args.input_file
    g = graphviz.Source.from_file(input_file, format='png')
    g.view()

if __name__ == '__main__':
    main()
