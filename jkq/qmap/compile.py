#
# This file is part of JKQ QMAP library which is released under the MIT license.
# See file README.md or go to http://iic.jku.at/eda/research/quantum_verification/ for more information.
#
import pickle
from pathlib import Path
from typing import Any, Dict, Union
from .pyqmap import map, Method, InitialLayoutStrategy, LayeringStrategy, Arch


def compile(circ, arch: Union[str, Arch],
            calibration: str = "",
            method: Method = Method.heuristic,
            initial_layout: InitialLayoutStrategy = InitialLayoutStrategy.dynamic,
            layering: LayeringStrategy = LayeringStrategy.individual_gates,
            use_teleportation: bool = False,
            teleportation_fake: bool = False,
            teleportation_seed: int = 0,
            save_mapped_circuit: bool = True,
            csv: bool = False,
            statistics: bool = False,
            verbose: bool = False
            ) -> Dict[str, Any]:
    """Interface to the JKQ QMAP tool for mapping quantum circuits

    :param circ: Path to first circuit file, path to Qiskit QuantumCircuit pickle, or Qiskit QuantumCircuit object
    :param arch: Path to architecture file or one of the available architectures (Arch)
    :type arch: Union[str, Arch]
    :param calibration: Path to file containing calibration information
    :param method: Mapping technique to use (*heuristic* | exact)
    :type method: Method
    :param initial_layout: Strategy to use for determining initial layout (only relevant for heuristic mapper)
    :type initial_layout: InitialLayoutStrategy
    :param layering: Circuit layering strategy to use (*individual_gates* | disjoint_qubits | odd_qubits | qubit_triangle)
    :type layering: LayeringStrategy
    :param use_teleportation:  Use teleportation in addition to swaps
    :param teleportation_fake: Assign qubits as ancillary for teleportation in the initial placement but don't actually use them (used for comparisons)
    :param teleportation_seed: Fix a seed for the RNG in the initial ancilla placement (0 means the RNG will be seeded from /dev/urandom/ or similar)
    :param save_mapped_circuit: Include .qasm string of the mapped circuit in result
    :type save_mapped_circuit: bool
    :param csv: Create CSV string for result
    :type csv: bool
    :param statistics: Print statistics
    :type statistics: bool
    :param verbose: Print more detailed information during the mapping process
    :type verbose: bool
    :return: JSON object containing results
    :rtype: Dict[str, Any]
    """

    if type(circ) == str and Path(circ).suffix == '.pickle':
        circ = pickle.load(open(circ, "rb"))

    result = map(circ, arch, {
        "calibration": calibration,
        "method": method.name,
        "initialLayout": initial_layout.name,
        "layering": layering.name,
        "use_teleportation": use_teleportation,
        "teleportation_fake": teleportation_fake,
        "teleportation_seed": teleportation_seed,
        "saveMappedCircuit": save_mapped_circuit,
        "csv": csv,
        "statistics": statistics,
        "verbose": verbose
    })

    if "error" in result:
        print(result["error"])

    return result
