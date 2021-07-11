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
	using Mapper::Mapper; // import constructors from parent class

	void map(const MappingSettings& ms) override;
	
	enum Ecc {
		Q3, Q9
	};

	struct Node {
		unsigned long costFixed = 0;
		double costHeur = 0.;
		double lookaheadPenalty = 0.;
		double costTotal = 0.;
		std::array<short, MAX_DEVICE_QUBITS> qubits{}; // get qubit at specific location
		std::array<short, MAX_DEVICE_QUBITS> locations{}; // get location of specific qubit
		bool done = true;
		std::vector<std::vector<Exchange>> swaps = {};
		unsigned long nswaps = 0;

		Node() = default;
		Node(const std::array<short, MAX_DEVICE_QUBITS>& q, const std::array<short, MAX_DEVICE_QUBITS>& loc, const std::vector<std::vector<Exchange>>& sw = {}) {
			std::copy(q.begin(), q.end(), qubits.begin());
			std::copy(loc.begin(), loc.end(), locations.begin());
			std::copy(sw.begin(), sw.end(), std::back_inserter(swaps));
		}


		void checkUnfinished(const Architecture& arch, const Gate& gate) {
			if (arch.distance(locations.at(gate.control), locations.at(gate.target)) > COST_DIRECTION_REVERSE) {
				done = false;
			}
		}

		std::ostream& print(std::ostream& out) const {
			out << "{\n";
			out << "\t\"done\": " << done << ",\n";
			out << "\t\"cost\": {\n";
			out << "\t\t\"fixed\": " << costFixed << ",\n";
			out << "\t\t\"heuristic\": " << costHeur << ",\n";
			out << "\t\t\"total\": " << costTotal << ",\n";
			out << "\t\t\"lookahead_penalty\": " << lookaheadPenalty << "\n";
			out << "\t},\n";
			out << "\t\"nswaps\": " << nswaps << "\n}\n";
			return out;
		}
	};

protected:
	unique_priority_queue<Node> nodes {};

	void initResults() override;

	void writeEncoding();

	void writeDecoding();

	double distanceOnArchitectureOfLogicalQubits(unsigned short control, unsigned short target) {
		return architecture.distance(locations.at(control), locations.at(target));
	}

	double distanceOnArchitectureOfPhysicalQubits(unsigned short control, unsigned short target) {
		return architecture.distance(control, target);
	}

private:
    void writeToffoli(unsigned short c1, unsigned short c2, unsigned short target);
    void writeCnot(unsigned short control, unsigned short target);

};


#endif //QMAP_EccMapper_HPP
