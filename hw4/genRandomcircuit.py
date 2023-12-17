import numpy as np
import sys
import random


from qiskit.circuit import Gate, QuantumCircuit, QuantumRegister
from qiskit.circuit.library import (
    CXGate,
    HGate,
    SGate,
    TGate,
)


def my_random_circuit(num_qubits, num_gates, t_count=0, seed=None):
    """Generate a pseudo random Clifford circuit."""

    clifford = ["cx", "h", "s"]

    instructions = {
        "cx": (CXGate(), 2),
        "s": (SGate(), 1),
        "h": (HGate(), 1),
        "t": (TGate(), 1)}

    if isinstance(seed, np.random.Generator):
        rng = seed
    else:
        rng = np.random.default_rng(seed)

    clifford_num = num_gates - t_count

    samples = rng.choice(clifford, clifford_num)


    for i in range(t_count): samples = np.append(samples, ["t"])
    random.shuffle(samples)


    circ = QuantumCircuit(num_qubits)

    for name in samples:
        gate, nqargs = instructions[name]
        qargs = rng.choice(range(num_qubits), nqargs, replace=False).tolist()
        circ.append(gate, qargs)

    return circ


if __name__ == "__main__":
    # Problem 3
        
    for i in range(15):
        for j in range(50):
            outputdir = "benchmark/t%i/case%i.qasm" %(i,j+1)
            qc = my_random_circuit(num_qubits=3, num_gates=100, t_count=i)
            qc.qasm(filename=outputdir)





