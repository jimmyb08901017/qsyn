/****************************************************************************
  PackageName  [ pp ]
  Synopsis     [ Implement matroid partition algorithm ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#pragma once

#include "util/boolean_matrix.hpp"

namespace qsyn::pp{

using Partition = dvlab::BooleanMatrix;
using Partitions = std::vector<dvlab::BooleanMatrix>;


struct Node{
    std::vector<Node*> next_nodes;
    Partition in_p;
    Partitions to_p;
};


class Matroid{

public:
    void add_into_partition(){};
    bool independant_oracle(){};

private:
    Partitions _partitions;
    std::vector<Node*> _graph;

};


} // namespace qsyn::Phase_Polynomial


