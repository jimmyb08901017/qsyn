/****************************************************************************
  PackageName  [ pp ]
  Synopsis     [ Implement matroid partition algorithm ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include "matroid.hpp"

#include <iostream>
#include "qcir/qcir.hpp"
#include "util/boolean_matrix.hpp"

using namespace qsyn::qcir;
using namespace std;

namespace qsyn::pp {

using Row    =  dvlab::BooleanMatrix::Row;

/**
 * @brief Set initial partition / graph
 * @param dvlab::BooleanMatrix poly
 * @param n: data qubit number
 * @param a: ancilla number
 * 
 */
void Matroid::initial(dvlab::BooleanMatrix poly, size_t n, size_t a){
    std::cout << "Initial a matroid" << endl;
    _qubit_num = n+a;
    _poly = poly;
    _variable = n;
    for(size_t i=0; i<_poly.num_rows(); i++){
      Node new_node;
      new_node.nth_row = i;
      _graph.emplace_back(std::move(new_node));
      // Matroid::add_into_partition(poly.get_row(i));
    }

}

/**
 * @brief Add a new poly term node into graph
 * @param dvlab::BooleanMatrix poly
 * 
 */
bool Matroid::add_into_partition(Node n){
    
    auto is_replaceable = [&](Node a, Node b){ // verify if a can be replace by b in its partition
        dvlab::BooleanMatrix temp = a.in_p;
        auto r = temp.find_row(_poly[a.nth_row]);
        assert (r.has_value());
        temp.erase_row(r.value());
        temp.push_row(_poly[b.nth_row]);
      
        return Matroid::independant_oracle(temp);
    };

    // auto is_acceptable_by_partition = [&](Node a, Partition p){ // verify if a node can be easily add to a partition
    //     Row r(_poly[a.nth_row]);
    //     p.push_row(r);
    //     return Matroid::independant_oracle(p);
    // };
    
    for_each(_graph.begin(), _graph.end(), [&](Node u){
      if (is_replaceable(u, n)){
        n.next_nodes.emplace_back(u);
        u.prev_nodes.emplace_back(n);
      } 
    });

    // auto path = Matroid::BFS(n);



    return true;
}

/**
 * @brief Use breadth first search to find a path to partition node.
 * @param Node head
 * @return vector<Node>: the path to partition node.
 */
// std::optional<Path> Matroid::BFS(Node head){
//     Matroid::reset_marks();

//     std::vector<Node> queue;

//     head.bfs_node.marked = true;
//     queue.emplace_back(std::move(head));

//     while(queue.size()){
//       Node* s = queue[0];
      
      
//       queue.erase(queue.begin());
//     }
    
//     return nullopt;
// }

/**
 * @brief Verify if satisfy Lemma: dim(V) - rank(S) <= n - |S|
 * @param Partition S
 * @return true if satified, ow, false
 */
bool Matroid::independant_oracle(Partition S){
    size_t rank = S.gaussian_elimination_skip(S.num_cols(), false);
    cout << "Rank is " << rank << ". Return " << (_variable-rank <= _qubit_num - S.num_rows()) << endl;
    return (_variable - rank <= _qubit_num - S.num_rows());
}


} // namespace qsyn::pp

