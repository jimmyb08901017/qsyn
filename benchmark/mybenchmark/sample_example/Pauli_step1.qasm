OPENQASM 2.0;
include "qelib1.inc";

qreg q[2];
cx q[0], q[1];
rzz(2*pi/10) q[0], q[1];
rz(pi/10) q[0];
rx(5*pi/10) q[1];
rx(pi*3/10) q[0];
rz(8*pi/10) q[0];