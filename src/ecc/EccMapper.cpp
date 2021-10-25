/*
 * This file is part of the JKQ QMAP library which is released under the MIT license.
 * See file README.md or go to https://iic.jku.at/eda/research/ibm_qx_mapping/ for more information.
 */

#include "ecc/EccMapper.hpp"
#include "Mapper.hpp"

#include <chrono>
//#include <stdlib.h>

EccMapper::EccMapper(struct EccInfo eccInfo, qc::QuantumComputation& quantumcomputation, bool doMeasure): ecc(eccInfo), qc(quantumcomputation), doMeasuring(doMeasure) {
}

void EccMapper::map() {
    const auto start = std::chrono::steady_clock::now();
	qc.stripIdleQubits(true, false);
	long nInputGates = 0;

	initResults();

	writeEccEncoding();

    for(auto& gate: qc) {
        nInputGates++;
        mapGate(gate);
        if(done) break;
    }

    if(!done) {
        writeEccDecoding();
        done=true;
    }
    const auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> diff = end - start;
	results.time = diff.count();
	results.timeout = false;
	results.input_gates = nInputGates;

	long nOutputGates = 0; //TODO count while creating?
    for(auto& gate: qcMapped) {
        nOutputGates++;
    }
    results.output_gates = nOutputGates;
}

std::ostream& EccMapper::printResult(std::ostream& out, bool printStatistics) {
    double gateRatio = results.output_gates;
    gateRatio/=results.input_gates;
    out << "\tused error correcting code: " << ecc.name << std::endl;
    out << "\tgate overhead: " << gateRatio << std::endl;
	out << "\tinput qubits: " << results.input_qubits << std::endl;
	out << "\tinput gates: " << results.input_gates << std::endl;
	out << "\toutput qubits: " << results.output_qubits << std::endl;
	out << "\toutput gates: " << results.output_gates << std::endl;
	return out;
}

void EccMapper::writeToffoli(unsigned short c1, bool pos1, unsigned short c2, bool pos2, unsigned short target) {
    const int nQubitsMapped = qcMapped.getNqubits();
    std::vector<qc::Control> controls;
    qc::Control con1 (c1, pos1 ? qc::Control::pos : qc::Control::neg);
    controls.push_back(con1);
    qc::Control con2 (c2, pos2 ? qc::Control::pos : qc::Control::neg);
    controls.push_back(con2);
    qcMapped.emplace_back<qc::StandardOperation>(nQubitsMapped, controls, target, qc::X);
}

void EccMapper::writeToffoliPhase(unsigned short c1, bool pos1, unsigned short c2, bool pos2, unsigned short target) {
    const int nQubitsMapped = qcMapped.getNqubits();
    std::vector<qc::Control> controls;
    qc::Control con1 (c1, pos1 ? qc::Control::pos : qc::Control::neg);
    controls.push_back(con1);
    qc::Control con2 (c2, pos2 ? qc::Control::pos : qc::Control::neg);
    controls.push_back(con2);
    qcMapped.emplace_back<qc::StandardOperation>(nQubitsMapped, controls, target, qc::Z);
}

void EccMapper::initResults() {

	results.input_name = qc.getName();
	results.input_qubits = qc.getNqubits();

	results.output_name = qc.getName() + "_mapped";
	results.output_qubits = qc.getNqubits()*ecc.nRedundantQubits;	//TODO remove if error case (no ECC) is handled correclty

	qcMapped.addQubitRegister(results.output_qubits);
	qcMapped.addClassicalRegister(qc.getNcbits()*(1+ecc.nMoreClassicals));
	results.method = Method::Ecc;
}

void EccMapper::writeCnot(unsigned short control, unsigned short target) {
    qcMapped.emplace_back<qc::StandardOperation>(qcMapped.getNqubits(), qc::Control{(signed char)control}, target, qc::X);
}


//-------------------------------------------------------------------------------------------------------------------

Q3ShorEccMapper::Q3ShorEccMapper(qc::QuantumComputation& qc, bool doMeasuring): EccMapper({Ecc::Q3Shor, doMeasuring?5:3,doMeasuring?2:0, Q3ShorEccMapper::getEccName()}, qc, doMeasuring) {}

void Q3ShorEccMapper::writeEccEncoding() {
	const int nQubits = qc.getNqubits();

    for(int i=0;i<nQubits;i++) {
        writeCnot(i, i+nQubits);
        writeCnot(i, i+2*nQubits);
    }
}

void Q3ShorEccMapper::writeEccDecoding() {
    if(doMeasuring) measureAndCorrect();
    const int nQubits = qc.getNqubits();
    for(int i=0;i<nQubits;i++) {
        writeCnot(i, i+nQubits);
        writeCnot(i, i+2*nQubits);
        writeToffoli(i+nQubits, true, i+2*nQubits, true, i);
    }
}

void Q3ShorEccMapper::measureAndCorrect() {
    const int nQubits = qc.getNqubits();
    for(int i=0;i<nQubits;i++) {

        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, i+3*nQubits, qc::H);
        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, i+4*nQubits, qc::H);

        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, qc::Control(i+3*nQubits, qc::Control::pos), i, qc::Z);
        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, qc::Control(i+3*nQubits, qc::Control::pos), i+nQubits, qc::Z);
        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, qc::Control(i+4*nQubits, qc::Control::pos), i+nQubits, qc::Z);
        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, qc::Control(i+4*nQubits, qc::Control::pos), i+2*nQubits, qc::Z);

        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, i+3*nQubits, qc::H);
        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, i+4*nQubits, qc::H);

        qcMapped.emplace_back<qc::NonUnitaryOperation>(nQubits*ecc.nRedundantQubits, i+3*nQubits, i);
        qcMapped.emplace_back<qc::NonUnitaryOperation>(nQubits*ecc.nRedundantQubits, i+4*nQubits, i+nQubits);

        writeToffoli(i+3*nQubits, true, i+4*nQubits, false, i);
        writeToffoli(i+3*nQubits, true, i+4*nQubits, true, i+nQubits);
        writeToffoli(i+3*nQubits, false, i+4*nQubits, true, i+2*nQubits);
    }
}

void Q3ShorEccMapper::mapGate(std::unique_ptr<qc::Operation> &gate) {
    const int nQubits = qc.getNqubits();
    switch(gate.get()->getType()) {
    case qc::I: break;
    case qc::Measure:
        writeEccDecoding();
        done=true;
        for(int nq = 0;nq<nQubits; nq++) {
            qcMapped.emplace_back<qc::NonUnitaryOperation>(nQubits*ecc.nRedundantQubits, nq, nq);
        }
        break;
    case qc::X:
    case qc::H:
    case qc::Y:
    case qc::Z:
    case qc::S:
    case qc::Sdag:
    case qc::T:
    case qc::Tdag:
        for(std::size_t j=0;j<gate.get()->getNtargets();j++) {
            auto i = gate.get()->getTargets()[j];
            if(gate.get()->getNcontrols()) {
                auto& ctrls = gate.get()->getControls();
                qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, ctrls, i, gate.get()->getType());
                std::vector<qc::Control> ctrls2, ctrls3;
                for(const auto &ct: ctrls) {
                    ctrls2.emplace_back(ct.qubit+nQubits, ct.type);
                    ctrls3.emplace_back(ct.qubit+2*nQubits, ct.type);
                }
                qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, ctrls2, i+nQubits, gate.get()->getType());
                qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, ctrls3, i+2*nQubits, gate.get()->getType());
            } else {
                qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, i, gate.get()->getType());
                qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, i+nQubits, gate.get()->getType());
                qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, i+2*nQubits, gate.get()->getType());
            }
        }
        break;
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
    case qc::ClassicControlled:
    default:
        throw QMAPException(gate.get()->getName());
    }
}

//--------------------------------------------------------------------------------------------------------------------------

Q9ShorEccMapper::Q9ShorEccMapper(qc::QuantumComputation& qc, bool doMeasuring) : EccMapper({Ecc::Q9Shor, doMeasuring?17:9,doMeasuring?8:0, Q9ShorEccMapper::getEccName()}, qc, doMeasuring) {
}

void Q9ShorEccMapper::writeEccEncoding() {
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

void Q9ShorEccMapper::writeEccDecoding() {
if(doMeasuring) measureAndCorrect();
    const int nQubits = qc.getNqubits();
    const int nQubitsMapped = qcMapped.getNqubits();
    for(int i=0;i<nQubits;i++) {
        writeCnot(i, i+nQubits);
        writeCnot(i, i+2*nQubits);
        writeToffoli(i+nQubits, true, i+2*nQubits, true, i);
        writeCnot(i+3*nQubits, i+4*nQubits);
        writeCnot(i+3*nQubits, i+5*nQubits);
        writeToffoli(i+4*nQubits, true, i+5*nQubits, true, i+3*nQubits);
        writeCnot(i+6*nQubits, i+7*nQubits);
        writeCnot(i+6*nQubits, i+8*nQubits);
        writeToffoli(i+7*nQubits, true, i+8*nQubits, true, i+6*nQubits);

        qcMapped.emplace_back<qc::StandardOperation>(nQubitsMapped, i, qc::H);
        qcMapped.emplace_back<qc::StandardOperation>(nQubitsMapped, i+3*nQubits, qc::H);
        qcMapped.emplace_back<qc::StandardOperation>(nQubitsMapped, i+6*nQubits, qc::H);

        writeCnot(i, i+3*nQubits);
        writeCnot(i, i+6*nQubits);
        writeToffoli(i+3*nQubits, true, i+6*nQubits, true, i);
    }
}

void Q9ShorEccMapper::mapGate(std::unique_ptr<qc::Operation> &gate) {
    const int nQubits = qc.getNqubits();
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
    case qc::Measure:
        writeEccDecoding();
        done=true;
        for(int nq = 0;nq<nQubits; nq++) {
            qcMapped.emplace_back<qc::NonUnitaryOperation>(nQubits*ecc.nRedundantQubits, nq, nq);
        }
        break;
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
    case qc::ClassicControlled:
    default:
        results.output_gates = -1;
        results.output_qubits = -1;
        throw QMAPException(gate.get()->getName());
    }

    for(std::size_t j=0;j<gate.get()->getNtargets() && !done;j++) {
        auto i = gate.get()->getTargets()[j];
        if(gate.get()->getNcontrols()) {
            auto& ctrls = gate.get()->getControls();
            for(int k=0;k<9;k++) {
                std::vector<qc::Control> ctrls2;
                for(const auto &ct: ctrls) {
                    qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, ct.qubit, qc::H);
                    ctrls2.emplace_back(ct.qubit+k*nQubits, ct.type);
                }
                qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, ctrls2, i+k*nQubits, type);
                for(const auto &ct: ctrls) {
                    qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, ct.qubit, qc::H);
                }
            }
        } else {
            for(int k=0;k<9;k++) {
                qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, i+k*nQubits, type);
            }
        }
    }

}

void Q9ShorEccMapper::measureAndCorrect() {
    const int nQubits = qc.getNqubits();
    for(int i=0;i<nQubits;i++) {
        //syntactic sugar for qubit indices
        unsigned int q[9];//qubits
        unsigned int a[8];//ancilla qubits
        qc::Control ca[8];//ancilla controls
        qc::Control cna[8];//negative ancilla controls
        unsigned int m[8];
        for(int j=0;j<9;j++) { q[j] = i+j*nQubits;}
        for(int j=0;j<8;j++) { a[j] = i+(j+9)*nQubits; m[j] = i+j*nQubits;}
        for(int j=0;j<8;j++) { ca[j] = qc::Control(a[j], qc::Control::pos);}
        for(int j=0;j<8;j++) { cna[j] = qc::Control(a[j], qc::Control::neg);}


        // PREPARE measurements --------------------------------------------------------
        for(int j=0;j<8;j++) {
            qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, a[j], qc::H);
        }
        //x errors = indirectly via controlled z
        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, ca[0], q[0], qc::Z);
        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, ca[0], q[1], qc::Z);
        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, ca[1], q[1], qc::Z);
        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, ca[1], q[2], qc::Z);

        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, ca[2], q[3], qc::Z);
        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, ca[2], q[4], qc::Z);
        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, ca[3], q[4], qc::Z);
        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, ca[3], q[5], qc::Z);

        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, ca[4], q[6], qc::Z);
        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, ca[4], q[7], qc::Z);
        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, ca[5], q[7], qc::Z);
        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, ca[5], q[8], qc::Z);

        //z errors = indirectly via controlled x/CNOT
        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, ca[6], q[0], qc::X);
        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, ca[6], q[1], qc::X);
        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, ca[6], q[2], qc::X);
        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, ca[6], q[3], qc::X);
        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, ca[6], q[4], qc::X);
        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, ca[6], q[5], qc::X);

        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, ca[7], q[3], qc::X);
        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, ca[7], q[4], qc::X);
        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, ca[7], q[5], qc::X);
        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, ca[7], q[6], qc::X);
        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, ca[7], q[7], qc::X);
        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, ca[7], q[8], qc::X);

        for(int j=0;j<8;j++) {
            qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, a[j], qc::H);
        }

        //MEASURE ancilla qubits
        for(int j=0;j<8;j++) {
            //qcMapped.measure(a[j], m[j]);
            qcMapped.emplace_back<qc::NonUnitaryOperation>(nQubits*ecc.nRedundantQubits, a[j], m[j]);
        }

        //CORRECT
        //x, i.e. bit flip errors
        writeToffoli(a[0], true, a[1], false, q[0]);
        writeToffoli(a[0], true, a[1], true, q[1]);
        writeToffoli(a[0], false, a[1], false, q[2]);

        writeToffoli(a[2], true, a[3], false, q[3]);
        writeToffoli(a[2], true, a[3], true, q[4]);
        writeToffoli(a[2], false, a[3], false, q[5]);

        writeToffoli(a[4], true, a[5], false, q[6]);
        writeToffoli(a[4], true, a[5], true, q[7]);
        writeToffoli(a[4], false, a[5], false, q[8]);

        //z, i.e. phase flip errors
        writeToffoliPhase(a[6], true, a[7], false, q[0]);
        writeToffoliPhase(a[6], true, a[7], true, q[3]);
        writeToffoliPhase(a[6], false, a[7], true, q[6]);

    }
}

//--------------------------------------------------------------------------------------------------------------------------

IdEccMapper::IdEccMapper(qc::QuantumComputation& qc, bool doMeasuring) : EccMapper({Ecc::Id, 1,0, IdEccMapper::getEccName()}, qc, doMeasuring) {
}

void IdEccMapper::measureAndCorrect() {}

void IdEccMapper::writeEccEncoding() {}

void IdEccMapper::writeEccDecoding() {}

void IdEccMapper::mapGate(std::unique_ptr<qc::Operation> &gate) {
    const int nQubits = qc.getNqubits();
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
        qcMapped.emplace_back<qc::StandardOperation>(nQubits*ecc.nRedundantQubits, i, gate.get()->getType());
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
    case qc::ClassicControlled:
    default:
        results.output_gates = -1;
        results.output_qubits = -1;
        throw QMAPException(gate.get()->getName());
    }
}

