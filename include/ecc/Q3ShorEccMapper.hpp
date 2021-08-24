#include "Mapper.hpp"
#include "EccMapper.hpp"

#ifndef QMAP_Q3ShorEccMapper_HPP
#define QMAP_Q3ShorEccMapper_HPP

class Q3ShorEccMapper: public EccMapper {
public:
    Q3ShorEccMapper(qc::QuantumComputation& qc, Architecture& architecture);

protected:
    void writeEncoding() override;

	void writeDecoding() override;
};

#endif //QMAP_Q3ShorEccMapper_HPP
