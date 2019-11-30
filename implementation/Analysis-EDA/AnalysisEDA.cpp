/*
 * AnalysisEDA.cpp
 *
 * This file contains the implementation of the simulator.
 */

#include "AnalysisEDA.h"
#include "Graph/GraphHandler.h"
#include "Utils-STA/Timing.h"
#include <iostream>
#include <map>
#include <string>
#include <algorithm>


Logic AnalysisEDA::AND(Logic a, Logic b) {
    if(a == Logic::logic0 || b == Logic::logic0) return Logic::logic0;
    else if(a == Logic::logicX || b == Logic::logicX) return Logic::logicX;
    else return Logic::logic1;
}

Logic AnalysisEDA::OR(Logic a, Logic b) {
    if(a == Logic::logic1 || b == Logic::logic1) return Logic::logic1;
    else if(a == Logic::logicX || b == Logic::logicX) return Logic::logicX;
    else return Logic::logic0;
}

Logic AnalysisEDA::NOT(Logic a) {
    if(a == Logic::logic0) return Logic::logic1;
    else if(a == Logic::logic1) return Logic::logic0;
    else return Logic::logicX;
}

Logic AnalysisEDA::getNetLogic(size_t net){
    if(true == is_net_done[net])
        return schaltung[net];
    else {
        const Element* in_element = graphHandler->getNet(net)->getInElement();
        for(const Net* net: graphHandler->getNet(net)->getInElement()->getInNets())
            schaltung[net->getId()] = getNetLogic(net->getId());

        is_net_done[net] = true;

        switch (graphHandler->getNet(net)->getInElement()->getElementInfo()->getType()) {
            case ElementType::And:
                return AND(schaltung[in_element->getInNets()[0]->getId()], schaltung[in_element->getInNets()[1]->getId()]);
            case ElementType::Or:
                return OR(schaltung[in_element->getInNets()[0]->getId()], schaltung[in_element->getInNets()[1]->getId()]);
            case ElementType::Not:
                return NOT(schaltung[in_element->getInNets()[0]->getId()]);
            default: break;
        }
    }
    return Logic::logicX;
}

void AnalysisEDA::run() {

    std::vector<size_t> outputPort;
    std::vector<size_t> inputPort;
    const DelayCalculator delayCalculator(graphHandler);
    std::cout << "test:" << delayCalculator.getClockPeriod() << std::endl;

    for(const Net* net :graphHandler->getAllNets()){
        schaltung[net->getId()] = Logic::logicX;
        is_net_done[net->getId()] = false;
        if (net->getInElement() == nullptr)
            inputPort.push_back(net->getId());
        else if(ElementType::Dff == net->getInElement()->getElementInfo()->getType())
            regDff[net->getId()] = Logic::logicX;
        if (net->getOutElements()[0] == nullptr)
            outputPort.push_back(net->getId());
    }
    for (auto i : outputPort)
        std::cout << graphHandler->getNet(i)->getName() << "\t";
    std::cout << std::endl;

    for (const std::vector<Logic>& timeStep : inputData) {
        unsigned long i = 0;

        for(auto iter = is_net_done.begin(); iter != is_net_done.end(); iter++)
            iter->second = false;

        for (const Logic& value : timeStep) {
            schaltung[inputPort[i]] = value;
            is_net_done[inputPort[i]] = true;
            i++;
        }

        for (auto i = regDff.begin(); i != regDff.end(); i++ ){
            schaltung[graphHandler->getNet(i->first)->getId()] = i->second;
            is_net_done[i->first] = true;
        }

        for(std::vector<size_t>::iterator i = outputPort.begin(); i != outputPort.end(); i++)
            std::cout << getNetLogic(*i) << "\t";
        std::cout << std::endl;

        for (auto i = regDff.begin(); i != regDff.end(); i++ )
            if("CLOCK" == graphHandler->getNet(i->first)->getInElement()->getInNets()[0]->getName())
                i->second  = getNetLogic(graphHandler->getNet(i->first)->getInElement()->getInNets()[1]->getId());
            else i->second  =  getNetLogic(graphHandler->getNet(i->first)->getInElement()->getInNets()[0]->getId());
    }

#if false
    /*
     * The following code shows some exemplary usage of the API
     */

    // Iterate all elements:
    for (const Element* element : graphHandler->getAllElements()) {
        std::cout << element->getName() << "\t" << element->getElementInfo()->getType()<< std::endl;
    }

    // Iterate all nets:
    for(const Net* net: graphHandler->getAllNets()) {
        std::cout << net->getName();
        if (net->getInElement() == nullptr)
            std::cout << " (primary input)";
        if (net->getOutElements()[0] == nullptr)
            std::cout << " (primary output)";
        std::cout << std::endl;
    }

    // Iterate all time steps:
    for (const std::vector<Logic>& timeStep : inputData) {
        for (const Logic& value : timeStep) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }
#endif
}
