#include "Q3ShorEccMapper.hpp"

Q3ShorEccMapper::Q3ShorEccMapper(qc::QuantumComputation& qc, Architecture& architecture): EccMapper(qc, architecture, Ecc::Q3, 3) {}

void Q3ShorEccMapper::writeEncoding() {
	const int nQubits = qc.getNqubits();
	const int nQubitsMapped = qcMapped.getNqubits();

    for(int i=0;i<nQubits;i++) {
        writeCnot(i, i+nQubits);
        writeCnot(i, i+2*nQubits);
    }
}

void Q3ShorEccMapper::writeDecoding() {
    const int nQubits = qc.getNqubits();
    const int nQubitsMapped = qcMapped.getNqubits();
    for(int i=0;i<nQubits;i++) {
        writeCnot(i, i+nQubits);
        writeCnot(i, i+2*nQubits);
        writeToffoli(i+nQubits, i+2*nQubits, i);
    }
}
