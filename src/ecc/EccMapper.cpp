/*
 * This file is part of the JKQ QMAP library which is released under the MIT license.
 * See file README.md or go to https://iic.jku.at/eda/research/ibm_qx_mapping/ for more information.
 */

#include "ecc/EccMapper.hpp"

#include <chrono>
//#include <stdlib.h>

void EccMapper::map(const MappingSettings& ms) {
	settings = ms;

	const auto start = std::chrono::steady_clock::now();
	qc.stripIdleQubits(true, false);
	const int nQubits = qc.getNqubits();

	initResults();

	createLayers();

	writeEncoding();

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
                qcMapped.emplace_back<qc::StandardOperation>(nQubits, i, gate.get()->getType());
                qcMapped.emplace_back<qc::StandardOperation>(nQubits, i+nQubits, gate.get()->getType());
                qcMapped.emplace_back<qc::StandardOperation>(nQubits, i+2*nQubits, gate.get()->getType());
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

    writeDecoding();

	const auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> diff = end - start;
	results.time = diff.count();
	results.timeout = false;
}

void EccMapper::writeEncoding() {
    const int nQubits = qc.getNqubits();
	for(int i=0;i<nQubits;i++) {
        writeCnot(i, i+nQubits);
        writeCnot(i, i+2*nQubits);
	}
}

void EccMapper::writeDecoding() {
    const int nQubits = qc.getNqubits();
    for(int i=0;i<nQubits;i++) {
        writeCnot(i, i+nQubits);
        writeCnot(i, i+2*nQubits);
        writeToffoli(i+nQubits, i+2*nQubits, i);
    }
}

void EccMapper::writeToffoli(unsigned short c1, unsigned short c2, unsigned short target) {
    const int nQubits = qc.getNqubits();
    qcMapped.emplace_back<qc::StandardOperation>(nQubits, target, qc::H);
    writeCnot(c2, target);
    qcMapped.emplace_back<qc::StandardOperation>(nQubits, target, qc::Tdag);
    writeCnot(c1, target);
    qcMapped.emplace_back<qc::StandardOperation>(nQubits, target, qc::T);
    writeCnot(c2, target);
    qcMapped.emplace_back<qc::StandardOperation>(nQubits, target, qc::Tdag);
    writeCnot(c1, target);
    qcMapped.emplace_back<qc::StandardOperation>(nQubits, target, qc::T);
    qcMapped.emplace_back<qc::StandardOperation>(nQubits, target, qc::H);
    qcMapped.emplace_back<qc::StandardOperation>(nQubits, c2, qc::Tdag);
    writeCnot(c1, c2);
    qcMapped.emplace_back<qc::StandardOperation>(nQubits, c2, qc::Tdag);
    writeCnot(c1, c2);
    qcMapped.emplace_back<qc::StandardOperation>(nQubits, c2, qc::S);
    qcMapped.emplace_back<qc::StandardOperation>(nQubits, c1, qc::T);
}

void EccMapper::initResults() {
	//Mapper::initResults();

	results.input_name = qc.getName();
	results.input_qubits = qc.getNqubits();
	//TODO remove architectural information, replaced by ECC
	results.architecture = architecture.getArchitectureName();
	results.calibration = architecture.getCalibrationName();
	results.layeringStrategy = settings.layeringStrategy;
	results.initialLayoutStrategy = settings.initialLayoutStrategy;

	results.output_name = qc.getName() + "_mapped";
	results.output_qubits = qc.getNqubits()*3;	//Q3

	qcMapped.addQubitRegister(results.output_qubits);
	results.method = Method::Ecc;

	results.seed = settings.teleportationSeed;
	results.input_teleportation_qubits = settings.teleportationQubits;
	results.output_teleportation_qubits = settings.teleportationQubits;
	results.output_teleportation_fake = settings.teleportationFake;
}

void EccMapper::writeCnot(unsigned short control, unsigned short target) {
    qcMapped.emplace_back<qc::StandardOperation>(qc.getNqubits(), qc::Control(control), target, qc::X);
}

