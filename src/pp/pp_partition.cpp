/****************************************************************************
  PackageName  [ pp ]
  Synopsis     [ Implement partition algorithm ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include "pp_partition.hpp"
#include <fmt/core.h>

#include <iostream>

#include "qcir/qcir.hpp"
#include "util/boolean_matrix.hpp"

using namespace qsyn::qcir;
using namespace std;

namespace qsyn::pp {

/**
 * @brief Set initial partition / graph
 * @param dvlab::BooleanMatrix poly
 * @param n: data qubit number
 * @param a: ancilla number
 *
 */
void Partitioning::initial(dvlab::BooleanMatrix poly, size_t n, size_t a) {
    _qubit_num = n + a;
    _poly      = poly;
    _variable  = n;
    _partitions.clear();
}

/**
 * @brief Verify if adding term t satisfies Lemma: dim(V) - rank(S) <= n - |S|
 * @param Partition S
 * @return true if satified, ow, false
 */
bool Partitioning::independent_oracle(Partition& S, term t) const {
    Partition temp = S;
    temp.push_row(t);
    size_t rank = temp.gaussian_elimination_skip(temp.num_cols(), true);
    cout << "Rank is " << rank << ". Return " << (_variable - rank <= _qubit_num - temp.num_rows()) << endl;
    return (_variable - rank <= _qubit_num - temp.num_rows());
}

/**
 * @brief Greedy partitioning
 * @param Wires w
 * @return Partitions
 */
Partitions Partitioning::greedy_partitioning(HMAP h_map, size_t rank) {
    for (auto [wires, q] : h_map) {
        Partitioning::greedy_partitioning_routine(_partitions, wires, rank);
    }
    if (_poly.num_rows() != 0) _poly.print_matrix();
    assert(_poly.num_rows() == 0);
    return _partitions;
}

/**
 * @brief Greedy partitioning routine call by Partitioning::greedy_partitioning
 * @param Partitions partitions: partitions to be added
 * @param Wires w: wires can be use now
 * @param size_t rank: rank of the wires (=data qubit number)
 */
void Partitioning::greedy_partitioning_routine(Partitions& partitions, Wires wires, size_t rank) {
    Partition p;
    std::vector<size_t> partitioned;

    // cout << "Rank is " << rank << endl;
    // cout << "Wires rank is " << wires.gaussian_elimination_skip(wires.num_cols(), true) << endl;
    // wires.print_matrix();

    auto is_constructable = [&](term t) {
        Wires temp = wires;
        temp.push_row(t);
        return (rank == temp.gaussian_elimination_skip(temp.num_cols(), true));
    };

    for (size_t i = 0; i < _poly.num_rows(); i++) {
        term r = _poly.get_row(i);
        // Wires temp = wires;
        // temp.push_row(r);
        // cout << "New term rank is " << temp.gaussian_elimination_skip(temp.num_cols(), true) << endl;
        if (!is_constructable(r)) continue;
        cout << "Is constructable" << endl;
        if (p.num_rows() == 0) {
            p.push_row(r);
            partitioned.emplace(partitioned.begin(), i);
            continue;
        }
        if (Partitioning::independent_oracle(p, r)) p.push_row(r);
        if (p.num_rows() == _qubit_num) {
            partitions.push_back(p);  // copy
            p.clear();
        }
        partitioned.emplace(partitioned.begin(), i);
    }
    cout << "Before size: " << _poly.num_rows() << endl;
    for_each(partitioned.begin(), partitioned.end(), [&](size_t i) { _poly.erase_row(i); });
    cout << "After size: " << _poly.num_rows() << endl;
}

/**
 * @brief Matriod partitioning
 * @param Wires w
 * @return Partitions
 */
Partitions Partitioning::matroid_partitioning(HMAP h_map) {
    cout << ">>> Start matroid partitioning..." << endl;
    for (auto [wires, q] : h_map) {
        Partitioning::matroid_partitioning_routine();
    }
    if (_poly.num_rows() != 0) _poly.print_matrix();
    assert(_poly.num_rows() == 0);
    cout << ">>> End matroid partitioning..." << endl;
    return _partitions;
}

void Partitioning::matroid_partitioning_routine() {
    cout << ">>>> Start matroid routine..." << endl;
    Matroid m(*this);
    for (auto& n : m._nodes) {
        m.add_into_partitions(n.get());
    }
}

/**
 * @brief Matriod Constructor
 * @param Partitioning& p
 * @return
 */
Partitioning::Matroid::Matroid(Partitioning& p) : master(p) {
    for (size_t i = 0, N = master._poly.get_matrix().size(); i < N; ++i) {
        auto newNode     = make_unique<Node>();
        newNode->nth_row = i;
        _nodes.push_back(std::move(newNode));
    }
}

bool Partitioning::Matroid::add_into_partitions(Node* n) {
    auto remove_from_partition = [&](Partition& p, Node* a) {  // remove node in a partition
        cout << a->nth_row << endl;
        master._poly[a->nth_row].print_row();
        p.print_matrix();
        auto r = p.find_row(master._poly[a->nth_row]);
        assert(r.has_value());  // not in partition
        p.erase_row(r.value());
        a->in_p = nullptr;
    };

    auto add_to_partition = [&](Partition* p, Node* a) {  // add node to a partition
        auto r = p->find_row(master._poly[a->nth_row]);
        assert(!r.has_value());  // in partition
        p->push_row(master._poly[a->nth_row]);
        a->in_p = p;
    };

    auto is_replaceable = [&remove_from_partition, this](Node* a, Node* b) {  // verify if a can be replace by b in its partition
        if (a->in_p == nullptr)
            return false;
        cout << "OK" << endl;
        a->in_p->print_matrix();
        auto partition = *(a->in_p);
        // auto partition2 = partition;
        // static_assert(&partition2 != &partition);

        // Because we are just checking if replaceable,
        // here we use the copy version of partition.
        assert(&partition != a->in_p);
        remove_from_partition(partition, a);
        // partition->push_row(master._poly[b->nth_row]);

        return master.independent_oracle(partition, master._poly[b->nth_row]);
    };

    // auto is_acceptable_by_partition = [&](Node* a, Partition& p){ // verify if a node can be easily add to a partition
    //     dvlab::BooleanMatrix::Row r(master._poly[a->nth_row]);
    //     p.push_row(r);
    //     return master.independent_oracle(p, master._poly[a->nth_row]);
    // };
    for (auto& u_ptr : _nodes) {
        auto u = u_ptr.get();
        assert(u);
        if (u == n) continue;
        if (is_replaceable(u, n)) {
            master.print_partitions();
            cout << "is_replaceable" << endl;
            n->next_nodes.emplace_back(u);
            u->prev_nodes.emplace_back(n);
        }
    }

    cout << "_nodes" << endl;
    for (auto& a: _nodes) {
        cout << a.get()->next_nodes.size() << " " << a.get()->prev_nodes.size() << endl;
    }

    // BFS
    cout << "Shortest Path" << endl;
    auto path = Matroid::find_shortest_path(n);

    if (!path.has_value()) {
        cout << "Add partition" << endl;
        Partition new_partition;
        new_partition.push_row(master.get_poly_row(n->nth_row));
        master._partitions.emplace_back(std::move(new_partition));
        new_partition.print_matrix();
        n->in_p = &new_partition;
        master.print_partitions();
        return false;
    } else {
        // Replace u with v
        cout << "recursive Replace" << endl;
        Path p         = path.value();
        auto prev_node = *p.begin();
        for (auto node = ++p.begin(); node != p.end(); ++node) {
            cout << "a" << endl;
            prev_node->in_p = (*node)->in_p;
            remove_from_partition((*node)->in_p, (*node));
            add_to_partition(prev_node->in_p, prev_node);

            prev_node = *node;
        }
        return true;
    }
}

/**
 * @brief Use breadth first search to find a path to partition node.
 * @param Node head
 * @return vector<Node*>: the path to partition node.
 */
std::optional<std::vector<Node*>> Partitioning::Matroid::find_shortest_path(Node* start) {
    reset_marks();
    Path path_end;

    // BFS
    std::vector<Node*> queue;
    queue.emplace_back(std::move(start));  // enqueue
    bool find = false;
    while (!queue.empty() & !find) {
        Node* t = queue.front();
        cout << t->nth_row << endl;
        queue.erase(queue.begin());  // dequeue

        for (auto A = master._partitions.begin(); A != master._partitions.end() && !find; ++A) {  // !find
            auto& row = master._poly[t->nth_row];
            auto r    = (*A).find_row(row);
            if (!r.has_value()) {  // if (Si != t.head_part())
                if (master.independent_oracle((*A), row)) {
                    // Traverse path and change all things accordingly
                    path_end.assign(t->path.begin(), t->path.end());
                    path_end.push_back(t);

                    find = true;
                } else {
                    for (auto& u : (*A).get_matrix()) {  // here u is row, not Node*
                        // Generate an path to the position before yi
                        // Take u out
                        auto tmp   = u;
                        auto index = (*A).find_row(u);  // this gets index of row
                        if(_nodes[index.value()].get()->marked)
                            continue;
                        (*A).erase_row(index.value());

                        if (master.independent_oracle((*A), row)) {
                            // Put u back in
                            (*A).push_row(u);

                            // Q.enqueue(u->t)
                            auto node = master._poly.find_row(u);
                            assert(node.has_value());
                            Node* leaf = _nodes[node.value()].get();
                            leaf->path.assign(t->path.begin(), t->path.end());
                            leaf->path.push_back(t);
                            queue.push_back(leaf);
                            _nodes[index.value()].get()->marked = true;
                        } else {
                            (*A).push_row(tmp);
                        }
                    }
                    // Remove CURRENT from Si
                    A->erase_row(r.value());
                }

            }
        }
    }
    queue.clear();

    cout << "path size: " << path_end.size() << endl;

    if (path_end.size() < 1)
        return nullopt;

    return path_end;
}

/**
 * @brief Reset node path for new usage.
 * @param
 * @return
 */
void Partitioning::Matroid::reset_marks() {
    for (auto& u_ptr : _nodes) {
        auto u = u_ptr.get();
        u->path.clear();
        u->marked = false;
    }
}

}  // namespace qsyn::pp
