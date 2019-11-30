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


#define OUTPUT false

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
//            case ElementType::Dff:
//                Logic new_value;
//                if (graphHandler->getNet(net)->getInElement()->getInNets()[0]->getName() == "CLOCK")
//                    new_value = schaltung[graphHandler->getNet(net)->getInElement()->getInNets()[1]->getId()];
//                else
//                    new_value = schaltung[graphHandler->getNet(net)->getInElement()->getInNets()[0]->getId()];
//                schaltung[net] = regDff[net];
//                regDff[net] = new_value;
//                return schaltung[net];
        default: break;
        }
    }
    return Logic::logicX;
}

void AnalysisEDA::getNetLogic() {
    std::map<size_t, bool> is_element_done;
    for (const Element* element : graphHandler->getAllElements())
        is_element_done[element->getId()] = false;

    while(1){
        for (const Element* element : graphHandler->getAllElements()) {
            if(is_element_done[element->getId()] == true) continue;

            switch (element->getElementInfo()->getType()) {
                case ElementType::And:
                    if(is_net_done[element->getInNets()[0]->getId()] == true && is_net_done[element->getInNets()[1]->getId()] == true){
                        schaltung[element->getOutNets()[0]->getId()] = AND(schaltung[element->getInNets()[0]->getId()], schaltung[element->getInNets()[1]->getId()]);
                        is_net_done[element->getOutNets()[0]->getId()] = true;
                        is_element_done[element->getId()] = true;
                    } break;
                case ElementType::Or:
                    if(is_net_done[element->getInNets()[0]->getId()] == true && is_net_done[element->getInNets()[1]->getId()] == true){
                        schaltung[element->getOutNets()[0]->getId()] = OR(schaltung[element->getInNets()[0]->getId()], schaltung[element->getInNets()[1]->getId()]);
                        is_net_done[element->getOutNets()[0]->getId()] = true;
                        is_element_done[element->getId()] = true;
                    } break;
                case ElementType::Not:
                    if(is_net_done[element->getInNets()[0]->getId()] == true){
                        schaltung[element->getOutNets()[0]->getId()] = NOT(schaltung[element->getInNets()[0]->getId()]);
                        is_net_done[element->getOutNets()[0]->getId()] = true;
                        is_element_done[element->getId()] = true;
                    } break;
                case ElementType::Dff:
                    if(is_net_done[element->getInNets()[0]->getId()] == true && is_net_done[element->getInNets()[1]->getId()] == true){
                        Logic new_value;
                        if (element->getInNets()[0]->getName() == "CLOCK")
                            new_value = schaltung[element->getInNets()[1]->getId()];
                        else
                            new_value = schaltung[element->getInNets()[0]->getId()];
                        schaltung[element->getOutNets()[0]->getId()] = regDff[element->getOutNets()[0]->getId()];
                        regDff[element->getOutNets()[0]->getId()] = new_value;
                        is_net_done[element->getOutNets()[0]->getId()] = true;
                        is_element_done[element->getId()] = true;
                    } break;
                default:
                    break;
            }
        }
        bool is_finished = true;
        for(std::map<size_t, bool>::iterator i = is_net_done.begin(); i != is_net_done.end(); i ++)
            if(i->second == false)  is_finished = false;
        if(is_finished == true) return;
    }
}

void AnalysisEDA::updateDffReg() {
    Logic new_value;

    for (std::map<size_t, Logic>::iterator i = regDff.begin(); i != regDff.end(); i++ ){
        schaltung[graphHandler->getNet(i->first)->getId()] = i->second;
        if("CLOCK" == graphHandler->getNet(i->first)->getInElement()->getInNets()[0]->getName())
            new_value = schaltung[graphHandler->getNet(i->first)->getInElement()->getInNets()[1]->getId()];
        else new_value =  schaltung[graphHandler->getNet(i->first)->getInElement()->getInNets()[0]->getId()];
        i->second = new_value;
    }
}

void AnalysisEDA::run() {

const DelayCalculator delayCalculator(graphHandler);
std::cout << "test:" << delayCalculator.getClockPeriod() << std::endl;


std::vector<size_t> outputPort;
std::vector<size_t> inputPort;

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


//for (std::vector<size_t>::iterator it = outputPort.begin(); it!= outputPort.end(); it++)
//    std::cout << graphHandler->getNet(*it)->getName() << "\t";
//std::cout << std::endl;

for (auto i : outputPort)
    std::cout << graphHandler->getNet(i)->getName() << "\t";
std::cout << std::endl;

for (const std::vector<Logic>& timeStep : inputData) {
    unsigned long i = 0;

    for(auto iter = is_net_done.begin(); iter != is_net_done.end(); iter++)
        iter->second = false;

    for (const Logic& value : timeStep) {
//        std::cout << value << " ";
        schaltung[inputPort[i]] = value;
        is_net_done[inputPort[i]] = true;
        i++;
    }

//    getNetLogic();

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

//std::map<size_t, Logic>::iterator iter;
//for (iter = schaltung.begin(); iter!= schaltung.end(); iter++) {
//    std::cout << graphHandler->getNet(iter->first)->getName() << " " << iter->second << std::endl;
//}

//std::cout << "input port:" <<std::endl;
//for (std::vector<size_t>::iterator it = inputPort.begin(); it!= inputPort.end(); it++) {
//    std::cout << graphHandler->getNet(*it)->getName() << std::endl;
//}


#if OUTPUT
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
