/*
 * This file is part of the JKQ QMAP library which is released under the MIT license.
 * See file README.md or go to https://iic.jku.at/eda/research/ibm_qx_mapping/ for more information.
 */

#include "QuantumComputation.hpp"
#include "MappingResults.hpp"
#include "utils.hpp"

#ifndef QMAP_EccMapper_HPP
#define QMAP_EccMapper_HPP

class EccMapper {
public:

    enum Ecc {
		Id, Q3Shor, Q9Shor
	};
    struct EccInfo {
        Ecc enumID;
        int nRedundantQubits;
        std::string name;
    };

    const struct EccInfo ecc;

	EccMapper(struct EccInfo ecc_type, qc::QuantumComputation& qc);
	virtual ~EccMapper() = default;

	void map();

    virtual std::ostream& printResult(std::ostream& out, bool printStatistics);

    //START copied from Mapper.hpp-----------------------------------------------------------------------------------------------
    virtual void dumpResult(const std::string& outputFilename) {
		if (qcMapped.empty()) {
			std::cerr << "Mapped circuit is empty." << std::endl;
			return;
		}

		size_t dot = outputFilename.find_last_of('.');
		std::string extension = outputFilename.substr(dot + 1);
		std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return ::tolower(c); });
		if (extension == "real") {
			dumpResult(outputFilename, qc::Real);
		} else if (extension == "qasm") {
			dumpResult(outputFilename, qc::OpenQASM);
		} else {
			throw QMAPException("[dump] Extension " + extension + " not recognized/supported for dumping.");
		}

	}

	virtual void dumpResult(const std::string& outputFilename, qc::Format format) {
		size_t slash = outputFilename.find_last_of('/');
		size_t dot = outputFilename.find_last_of('.');
		results.output_name = outputFilename.substr(slash+1, dot-slash-1);
		qcMapped.dump(outputFilename, format);
	}

	virtual void dumpResult(std::ostream& os, qc::Format format) {
		qcMapped.dump(os, format);
	}

	//END copied from Mapper.hpp-----------------------------------------------------------------------------------------------


protected:

    qc::QuantumComputation& qc;
	qc::QuantumComputation qcMapped;
	MappingResults results{};

	void initResults();

	virtual void writeEccEncoding()=0;

	virtual void writeEccDecoding()=0;

	virtual void mapGate(std::unique_ptr<qc::Operation> &gate)=0;

    void writeToffoli(unsigned short c1, unsigned short c2, unsigned short target);
    void writeCnot(unsigned short control, unsigned short target);

};


#endif //QMAP_EccMapper_HPP

#ifndef QMAP_Q3ShorEccMapper_HPP
#define QMAP_Q3ShorEccMapper_HPP

class Q3ShorEccMapper: public EccMapper {
public:
    Q3ShorEccMapper(qc::QuantumComputation& qc);

    static const std::string getEccName() {
        return "Q3Shor";
    }

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
    Q9ShorEccMapper(qc::QuantumComputation& qc);

    static const std::string getEccName() {
        return "Q9Shor";
    }

protected:
    void writeEccEncoding();

	void writeEccDecoding();

	void mapGate(std::unique_ptr<qc::Operation> &gate) override;
};

#endif //QMAP_Q9ShorEccMapper_HPP

#ifndef QMAP_IdEccMapper_HPP
#define QMAP_IdEccMapper_HPP

class IdEccMapper: public EccMapper {
public:
    IdEccMapper(qc::QuantumComputation& qc);

    static const std::string getEccName() {
        return "Id";
    }

protected:
    void writeEccEncoding();

	void writeEccDecoding();

	void mapGate(std::unique_ptr<qc::Operation> &gate) override;
};

#endif //QMAP_IdEccMapper_HPP


