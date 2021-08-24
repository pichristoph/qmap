#include "Mapper.hpp"
#include "EccMapper.hpp"

#ifndef QMAP_Q9ShorEccMapper_HPP
#define QMAP_Q9ShorEccMapper_HPP

class Q9ShorEccMapper: public EccMapper {
public:
    Q9ShorEccMapper(qc::QuantumComputation& qc, Architecture& architecture);

protected:
    void writeEncoding();

	void writeDecoding();
};

#endif //QMAP_Q9ShorEccMapper_HPP

