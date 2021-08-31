/*
 * This file is part of the JKQ QMAP library which is released under the MIT license.
 * See file README.md or go to https://iic.jku.at/eda/research/ibm_qx_mapping/ for more information.
 */

#include <cmath>
#include <iostream>
#include <boost/program_options.hpp>

#include "ecc/EccMapper.hpp"
#include "Architecture.hpp"

int main(int argc, char** argv) {
    namespace po = boost::program_options;
    po::options_description description("JKQ QMAP ecc mapper by https://iic.jku.at/eda/quantum -- Options");
    description.add_options()
            ("help,h", "produce help message")
            ("in", po::value<std::string>()->required(), "File to read from")
            ("out", po::value<std::string>()->required(), "File to write to")
            ("ecc", po::value<std::string>()->required(), "Error correcting code to use")
            ("ps", "print statistics")
            ("verbose", "Increase verbosity and output additional information to stderr")
            ;
    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, description), vm);
        if (vm.count("help")) {
            std::cout << description;
            return 0;
        }
        po::notify(vm);
    } catch (const po::error &e) {
        std::cerr << "[ERROR] " << e.what() << "! Try option '--help' for available commandline options.\n";
        std::exit(1);
    }


	const std::string circuit = vm["in"].as<std::string>();
	qc::QuantumComputation qc{};
	try {
		qc.import(circuit);
	} catch (std::exception const& e) {
		std::stringstream ss{};
		ss << "Could not import circuit: " << e.what();
		std::cerr << ss.str() << std::endl;
		std::exit(1);
	}

    EccMapper *mapper = nullptr;

    const std::string eccName = vm["ecc"].as<std::string>();

    if(eccName.compare(Q3ShorEccMapper::getEccName())==0) {
        mapper = new Q3ShorEccMapper(qc);
    } else if(eccName.compare(Q9ShorEccMapper::getEccName())==0) {
        mapper = new Q9ShorEccMapper(qc);
    } else {
        std::cerr << "No ECC found for " << eccName << std::endl;
        std::cerr << "Available ECCs: ";
        std::cerr << Q3ShorEccMapper::getEccName() << ", ";
        std::cerr << Q9ShorEccMapper::getEccName() << std::endl;
        std::exit(1);
    }


    mapper->map();

	mapper->dumpResult(vm["out"].as<std::string>());

	mapper->printResult(std::cout, vm.count("ps"));

	delete mapper;
}
