OPENQASM 2.0;
include "qelib1.inc";

qreg q[2];
rz(pi/10) q[0];
rz(2*pi/10) q[1];
cx q[0],q[1];
rx(3*pi/10) q[0];
cx q[0],q[1];
rz(4*pi/10) q[0];
rx(5*pi/10) q[1];
cx q[0],q[1];
rz(4*pi/10) q[0];