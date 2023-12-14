OPENQASM 2.0;
include "qelib1.inc";

qreg q[2];
rz(pi/10) q[0];
rz(2*pi/10) q[1];
ry(3*pi/10) q[0];
s q[0];
cx q[0], q[1];
ry(4*pi/10) q[0];
cx q[0], q[1];
rx(pi/2) q[1];
cx q[0], q[1];
ry(6*pi/10) q[0];
cx q[0], q[1];
rz(7*pi/10) q[0];
ry(8*pi/10) q[1];
