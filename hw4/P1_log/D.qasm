OPENQASM 2.0;
include "qelib1.inc";
qreg q[7];
cx q[1], q[5];
cz q[1], q[2];
p(3*pi/4) q[5];
cz q[0], q[5];
cx q[5], q[0];
h q[0];
t q[0];
cx q[1], q[0];
cx q[1], q[5];
t q[5];
tdg q[0];
cx q[5], q[0];
h q[5];
t q[5];
cx q[5], q[3];
tdg q[3];
cz q[1], q[3];
cx q[5], q[3];
cz q[2], q[3];
cx q[2], q[5];
tdg q[5];
cz q[1], q[5];
cx q[3], q[5];
t q[5];
cx q[2], q[5];
h q[5];
t q[5];
cx q[0], q[5];
cx q[1], q[0];
tdg q[5];
cx q[0], q[5];
cx q[1], q[0];
tdg q[5];
cx q[0], q[5];
t q[5];
cx q[0], q[5];
h q[5];
tdg q[5];
cx q[2], q[5];
t q[5];
cx q[3], q[5];
tdg q[5];
cx q[2], q[5];
cx q[2], q[3];
cx q[3], q[2];
cx q[2], q[3];
t q[5];
cz q[1], q[5];
cx q[5], q[0];
h q[5];
h q[0];
cx q[0], q[1];
cx q[1], q[0];
cx q[0], q[1];
cx q[1], q[5];
cx q[5], q[1];
cx q[1], q[5];