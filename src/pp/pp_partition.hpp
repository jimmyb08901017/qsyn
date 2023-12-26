/****************************************************************************
  PackageName  [ pp ]
  Synopsis     [ Implement partition algorithm ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#pragma once

#include <iostream>
#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include "util/boolean_matrix.hpp"

namespace qsyn::pp {

using Partition = dvlab::BooleanMatrix;
using term      = dvlab::BooleanMatrix::Row;
// using Partitions = std::vector<unique_ptr<Partition>>;
using Partitions = std::vector<Partition>;
using Wires      = dvlab::BooleanMatrix;
using HMAP       = std::vector<std::pair<dvlab::BooleanMatrix, size_t>>;

struct Node {
    size_t nth_row;
    std::vector<Node*> next_nodes;
    std::vector<Node*> prev_nodes;
    Partition* in_p;
    Partition* to_p;
    std::vector<Node*> path;
    bool marked;
};

using Path  = std::vector<Node*>;
using Nodes = std::vector<std::unique_ptr<Node>>;

/**
 * There are two partition methods:
 *     greedy,
 *     matroid
 */
class Partitioning {
    friend class Matriod;

    class Matroid {
        friend class Partitioning;

    public:
        Matroid(Partitioning& p);

        // void initial(dvlab::BooleanMatrix poly, size_t n, size_t a);
        void reset_marks();

        bool add_into_partitions(Node* node);
        std::optional<std::vector<Node*>> find_shortest_path(Node* head);

    private:
        Partitioning& master;
        Nodes _nodes;
    };

public:
    Partitioning(){};
    Partitioning(dvlab::BooleanMatrix poly, size_t n, size_t a) : _variable(n), _poly(poly) { _qubit_num = n + a; };

    void initial(dvlab::BooleanMatrix poly, size_t n, size_t a);
    bool independent_oracle(Partition&, term) const;  // check lemma 2

    // Greedy partitiion
    Partitions greedy_partitioning(HMAP h_map, size_t rank);
    void greedy_partitioning_routine(Partitions& partitions, Wires wires, size_t rank);

    // Matroid partition
    Partitions matroid_partitioning(HMAP h_map);
    void matroid_partitioning_routine();

    // Partition function


    // Get function
    Partitions get_partitions() const { return _partitions; }
    dvlab::BooleanMatrix::Row const& get_poly_row(size_t n) { return _poly.get_row(n); }

    // Print function
    void print_partitions() const {
        for (const auto& p: _partitions ){
            std::cout << &p << std::endl;
            p.print_matrix();
            fmt::println("");
        }
    }
    // void print_partition(spdlog::level::level_enum lvl = spdlog::level::level_enum::off) const {};

private:
    size_t _variable;
    size_t _qubit_num;
    Partitions _partitions;
    dvlab::BooleanMatrix _poly;
};

}  // namespace qsyn::pp
