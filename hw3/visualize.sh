#! /usr/bin/env bash
if [[ $# -eq 2 ]]; then
    ./parser $1 $2
    python visualize.py $2
else
    ./parser $1
    python visualize.py "AST_graph.gv"
fi
