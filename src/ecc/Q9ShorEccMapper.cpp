#include "ecc/Q9ShorEccMapper.hpp"
#include "ecc/EccMapper.hpp"

Q9ShorEccMapper::Q9ShorEccMapper(qc::QuantumComputation& qc, Architecture& architecture) : EccMapper(qc, architecture, Ecc::Q9) {
}

void Q9ShorEccMapper::writeEncoding() {
	const int nQubits = qc.getNqubits();
	const int nQubitsMapped = qcMapped.getNqubits();
    for(int i=0;i<nQubits;i++) {
        writeCnot(i, i+3*nQubits);
        writeCnot(i, i+6*nQubits);
        qcMapped.emplace_back<qc::StandardOperation>(nQubitsMapped, i, qc::H);
        qcMapped.emplace_back<qc::StandardOperation>(nQubitsMapped, i+3*nQubits, qc::H);
        qcMapped.emplace_back<qc::StandardOperation>(nQubitsMapped, i+6*nQubits, qc::H);
        writeCnot(i, i+nQubits);
        writeCnot(i, i+2*nQubits);
        writeCnot(i+3*nQubits, i+4*nQubits);
        writeCnot(i+3*nQubits, i+5*nQubits);
        writeCnot(i+6*nQubits, i+7*nQubits);
        writeCnot(i+6*nQubits, i+8*nQubits);
    }
}

void Q9ShorEccMapper::writeDecoding() {
    const int nQubits = qc.getNqubits();
    const int nQubitsMapped = qcMapped.getNqubits();
    for(int i=0;i<nQubits;i++) {
        writeCnot(i, i+nQubits);
        writeCnot(i, i+2*nQubits);
        writeToffoli(i+nQubits, i+2*nQubits, i);
        writeCnot(i+3*nQubits, i+4*nQubits);
        writeCnot(i+3*nQubits, i+5*nQubits);
        writeToffoli(i+4*nQubits, i+5*nQubits, i+3*nQubits);
        writeCnot(i+6*nQubits, i+7*nQubits);
        writeCnot(i+6*nQubits, i+8*nQubits);
        writeToffoli(i+7*nQubits, i+8*nQubits, i+6*nQubits);

        qcMapped.emplace_back<qc::StandardOperation>(nQubitsMapped, i, qc::H);
        qcMapped.emplace_back<qc::StandardOperation>(nQubitsMapped, i+3*nQubits, qc::H);
        qcMapped.emplace_back<qc::StandardOperation>(nQubitsMapped, i+6*nQubits, qc::H);

        writeCnot(i, i+3*nQubits);
        writeCnot(i, i+6*nQubits);
        writeToffoli(i+3*nQubits, i+6*nQubits, i);
    }
}
