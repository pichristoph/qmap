/*
 * This file is part of the JKQ QMAP library which is released under the MIT license.
 * See file README.md or go to https://iic.jku.at/eda/research/ibm_qx_mapping/ for more information.
 */

#include "Mapper.hpp"

#ifndef QMAP_EccMapper_HPP
#define QMAP_EccMapper_HPP

class EccMapper: public Mapper {
public:

    enum Ecc {
		Q3, Q9
	};

	EccMapper(qc::QuantumComputation& qc, Architecture& architecture, Ecc ecc_type, int nRedundantQubits);

	void map(const MappingSettings& ms) override;
    void map();


protected:

	void initResults() override;

	virtual void writeEccEncoding()=0;

	virtual void writeEccDecoding()=0;

	virtual void mapGate(std::unique_ptr<qc::Operation> &gate)=0;

	int nRedundantQubits;

    void writeToffoli(unsigned short c1, unsigned short c2, unsigned short target);
    void writeCnot(unsigned short control, unsigned short target);

    Ecc ecc;
};


#endif //QMAP_EccMapper_HPP

#ifndef QMAP_Q3ShorEccMapper_HPP
#define QMAP_Q3ShorEccMapper_HPP

class Q3ShorEccMapper: public EccMapper {
public:
    Q3ShorEccMapper(qc::QuantumComputation& qc, Architecture& architecture);

protected:
    void writeEccEncoding() override;

	void writeEccDecoding() override;

	void mapGate(std::unique_ptr<qc::Operation> &gate) override;
};

#endif //QMAP_Q3ShorEccMapper_HPP

#ifndef QMAP_Q9ShorEccMapper_HPP
#define QMAP_Q9ShorEccMapper_HPP

class Q9ShorEccMapper: public EccMapper {
public:
    Q9ShorEccMapper(qc::QuantumComputation& qc, Architecture& architecture);

protected:
    void writeEccEncoding();

	void writeEccDecoding();

	void mapGate(std::unique_ptr<qc::Operation> &gate) override;
};

#endif //QMAP_Q9ShorEccMapper_HPP


