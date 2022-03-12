/*
 * AnalysisEDA.h
 *
 *  Created on: Jun 16, 2017
 *      Author: kuswik
 */

#ifndef AnalysisEDA_H_
#define AnalysisEDA_H_

#include <vector>
#include <map>
#include "template.h"
#include "Utils-STA/DelayCalculator.h"

// forward declarations
class GraphHandler;

class AnalysisEDA {
public:
    AnalysisEDA(const GraphHandler* graphHandler, const std::vector<std::vector<Logic>>& inputData) :
        graphHandler(graphHandler), inputData(inputData) {
    }
    virtual ~AnalysisEDA() {
    }

    void run();


private:
    // prevent from using the default constructor, copy constructor and assignment operator
    AnalysisEDA();
    AnalysisEDA(const AnalysisEDA&);
    AnalysisEDA& operator=(const AnalysisEDA&);

    Logic AND(Logic a, Logic b);
    Logic OR(Logic a, Logic b);
    Logic NOT(Logic a);
    Logic calLogicResult(size_t net);

    std::map<size_t, Logic> mSchaltung;
    std::map<size_t, bool> mNetMap;
    std::map<size_t, Logic> mRegDff;

    const GraphHandler* graphHandler;
    const std::vector<std::vector<Logic>>& inputData;
};

#endif /* AnalysisEDA_H_ */
