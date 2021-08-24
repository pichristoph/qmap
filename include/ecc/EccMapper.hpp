/*
 * This file is part of the JKQ QMAP library which is released under the MIT license.
 * See file README.md or go to https://iic.jku.at/eda/research/ibm_qx_mapping/ for more information.
 */

#include "Mapper.hpp"
#include "heuristic/unique_priority_queue.hpp"

#ifndef QMAP_EccMapper_HPP
#define QMAP_EccMapper_HPP

class EccMapper: public Mapper {
public:

    enum Ecc {
		Q3, Q9
	};

	EccMapper(qc::QuantumComputation& qc, Architecture& architecture, Ecc ecc_type, int nRedundantQubits);

	void map(const MappingSettings& ms) override;



protected:

	void initResults() override;

	virtual void writeEncoding()=0;

	virtual void writeDecoding()=0;

	virtual int nRedundantQubits;

	/*double distanceOnArchitectureOfLogicalQubits(unsigned short control, unsigned short target) {
		return architecture.distance(locations.at(control), locations.at(target));
	}

	double distanceOnArchitectureOfPhysicalQubits(unsigned short control, unsigned short target) {
		return architecture.distance(control, target);
	}*/

    void writeToffoli(unsigned short c1, unsigned short c2, unsigned short target);
    void writeCnot(unsigned short control, unsigned short target);

    Ecc ecc;
};


#endif //QMAP_EccMapper_HPP
