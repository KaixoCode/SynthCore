#include "Kaixo/Core/Processing/Processor.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------
    
    void Processor::receiveParameterValue(ParamID id, ParamValue value) {
        if (m_ParameterValues[id] != value) {
            m_ParameterValues[id] = value;
            param(id, value);
        }
    }

    // ------------------------------------------------
}