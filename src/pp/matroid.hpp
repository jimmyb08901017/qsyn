/****************************************************************************
  PackageName  [ pp ]
  Synopsis     [ Implement matroid partition algorithm ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#pragma once

#include "util/boolean_matrix.hpp"

namespace qsyn::pp{

using Partition   = dvlab::BooleanMatrix;
using term        = dvlab::BooleanMatrix::Row;
using Partitions  = std::vector<dvlab::BooleanMatrix>;


struct Node{
    size_t nth_row;
    std::vector<Node> next_nodes;
    std::vector<Node> prev_nodes;
    Partition in_p;
    Partitions to_p;
    struct BFS_node{
        bool marked;
        Node* parent;
    }bfs_node;
};



using Path        = std::vector<Node>;

class Matroid{

public:

    Matroid (){};
    Matroid (dvlab::BooleanMatrix poly, size_t n, size_t a){Matroid::initial(poly, n, a);};


    void initial(dvlab::BooleanMatrix poly, size_t n, size_t a);
    bool add_into_partition(Node);
    bool independant_oracle(Partition);

    // graph operation
    
    std::optional<Path> BFS(Node);
    void reset_marks(){for_each(_graph.begin(), _graph.end(), [](Node n){n.bfs_node.marked=false;n.bfs_node.parent=nullptr;});};

    // Get function
    Partitions get_partitions(){return _partitions;}

    // Print function
    // void print_partitions(spdlog::level::level_enum lvl = spdlog::level::level_enum::off) const {};
    // void print_partition(spdlog::level::level_enum lvl = spdlog::level::level_enum::off) const {};

private:
    size_t _variable;
    size_t _qubit_num;
    Partitions _partitions;
    std::vector<Node> _graph;
    dvlab::BooleanMatrix _poly;

};


} // namespace qsyn::pp


