/*
 * This file is part of the JKQ QMAP library which is released under the MIT license.
 * See file README.md or go to https://iic.jku.at/eda/research/ibm_qx_mapping/ for more information.
 */

#include "ecc/EccMapper.hpp"
#include "Mapper.hpp"

#include <chrono>
//#include <stdlib.h>

EccMapper::EccMapper(qc::QuantumComputation& qc, Architecture& architecture, Ecc ecc_type, int nRedQubits): Mapper(qc, architecture) {
    ecc=ecc_type;
    nRedundantQubits = nRedQubits;
}

void EccMapper::map(const MappingSettings& ms) {
	settings = ms;

	//TODO currently, specify ECC here.
	ecc = Ecc::Q3;
	//ecc = Ecc::Q9;

	const auto start = std::chrono::steady_clock::now();
	qc.stripIdleQubits(true, false);
	const int nQubits = qc.getNqubits();

	initResults();

	createLayers();

	writeEncoding();

	if(ecc==Ecc::Q3) {
    	for(auto& gate: qc) {
        	int i;
        	switch(gate.get()->getType()) {
            case qc::I: break;
            case qc::X:
            case qc::H:
            case qc::Y:
            case qc::Z:
            //TODO check S, T, V
            case qc::S:
            case qc::Sdag:
            case qc::T:
            case qc::Tdag:
            case qc::V:
            case qc::Vdag:
                //TODO controlled/multitarget check
                i = gate.get()->getTargets()[0];
                qcMapped.emplace_back<qc::StandardOperation>(nQubits*3, i, gate.get()->getType());
                qcMapped.emplace_back<qc::StandardOperation>(nQubits*3, i+nQubits, gate.get()->getType());
                qcMapped.emplace_back<qc::StandardOperation>(nQubits*3, i+2*nQubits, gate.get()->getType());
                break;

            case qc::U3:
            case qc::U2:
            case qc::Phase:
            case qc::SX:
            case qc::SXdag:
            case qc::RX:
            case qc::RY:
            case qc::RZ:
            case qc::SWAP:
            case qc::iSWAP:
            case qc::Peres:
            case qc::Peresdag:
            case qc::Compound:
		    case qc::ClassicControlled:break;
		    default: throw QMAPException("Gate not possible to encode in error code!");
        	}
    	}
    } else if(ecc==Ecc::Q9) {
    	for(auto& gate: qc) {
        	int i;
        	auto type = qc::I;
        	switch(gate.get()->getType()) {
            case qc::I: break;
            case qc::X:
            	type = qc::Z; break;
            case qc::H:
            	type = qc::H; break;
            case qc::Y:
            	type = qc::Y; break;
            case qc::Z:
            	type = qc::X; break;

            //TODO check S, T, V
            case qc::S:
            case qc::Sdag:
            case qc::T:
            case qc::Tdag:
            case qc::V:
            case qc::Vdag:
            case qc::U3:
            case qc::U2:
            case qc::Phase:
            case qc::SX:
            case qc::SXdag:
            case qc::RX:
            case qc::RY:
            case qc::RZ:
            case qc::SWAP:
            case qc::iSWAP:
            case qc::Peres:
            case qc::Peresdag:
            case qc::Compound:
		    case qc::ClassicControlled:break;
		    default: throw QMAPException("Gate not possible to encode in error code!");
        	}
        	//TODO controlled/multitarget check
            i = gate.get()->getTargets()[0];
            for(int j=0;j<9;j++) {
            	qcMapped.emplace_back<qc::StandardOperation>(nQubits*9, i+j*nQubits, type);
            }
    	}
    }

    writeDecoding();

	const auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> diff = end - start;
	results.time = diff.count();
	results.timeout = false;
}

void EccMapper::writeEncoding() {
	const int nQubits = qc.getNqubits();
	const int nQubitsMapped = qcMapped.getNqubits();
	if(ecc==Ecc::Q3) {
		for(int i=0;i<nQubits;i++) {
    	    writeCnot(i, i+nQubits);
    	    writeCnot(i, i+2*nQubits);
		}
	} else if(ecc==Ecc::Q9) {
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
}

void EccMapper::writeDecoding() {
    const int nQubits = qc.getNqubits();
    const int nQubitsMapped = qcMapped.getNqubits();
    if(ecc==Ecc::Q3) {
    	for(int i=0;i<nQubits;i++) {
    	    writeCnot(i, i+nQubits);
    	    writeCnot(i, i+2*nQubits);
     	   writeToffoli(i+nQubits, i+2*nQubits, i);
    	}
    } else if(ecc==Ecc::Q9) {
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
}

void EccMapper::writeToffoli(unsigned short c1, unsigned short c2, unsigned short target) {
    const int nQubits = qc.getNqubits();
    const int nQubitsMapped = qcMapped.getNqubits();
    qcMapped.emplace_back<qc::StandardOperation>(nQubitsMapped, target, qc::H);
    writeCnot(c2, target);
    qcMapped.emplace_back<qc::StandardOperation>(nQubitsMapped, target, qc::Tdag);
    writeCnot(c1, target);
    qcMapped.emplace_back<qc::StandardOperation>(nQubitsMapped, target, qc::T);
    writeCnot(c2, target);
    qcMapped.emplace_back<qc::StandardOperation>(nQubitsMapped, target, qc::Tdag);
    writeCnot(c1, target);
    qcMapped.emplace_back<qc::StandardOperation>(nQubitsMapped, target, qc::T);
    qcMapped.emplace_back<qc::StandardOperation>(nQubitsMapped, target, qc::H);
    qcMapped.emplace_back<qc::StandardOperation>(nQubitsMapped, c2, qc::Tdag);
    writeCnot(c1, c2);
    qcMapped.emplace_back<qc::StandardOperation>(nQubitsMapped, c2, qc::Tdag);
    writeCnot(c1, c2);
    qcMapped.emplace_back<qc::StandardOperation>(nQubitsMapped, c2, qc::S);
    qcMapped.emplace_back<qc::StandardOperation>(nQubitsMapped, c1, qc::T);
}

void EccMapper::initResults() {
	//Mapper::initResults();

	results.input_name = qc.getName();
	results.input_qubits = qc.getNqubits();

	results.output_name = qc.getName() + "_mapped";
	results.output_qubits = qc.getNqubits()*3;	//TODO remove if error case (no ECC) is handled correclty
	if(ecc==Ecc::Q3) {
		results.output_qubits = qc.getNqubits()*3;
	} else if(ecc==Ecc::Q9) {
		results.output_qubits = qc.getNqubits()*9;
	}


	qcMapped.addQubitRegister(results.output_qubits);
	results.method = Method::Ecc;
}

void EccMapper::writeCnot(unsigned short control, unsigned short target) {
    qcMapped.emplace_back<qc::StandardOperation>(qcMapped.getNqubits(), qc::Control(control), target, qc::X);
}

