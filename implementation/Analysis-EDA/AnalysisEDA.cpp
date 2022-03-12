/*
 * AnalysisEDA.cpp
 *
 * This file contains the implementation of the simulator.
 */

#include "AnalysisEDA.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <string>

#include "Graph/GraphHandler.h"

void AnalysisEDA::run() {
    std::vector<size_t> outputPort;
    std::vector<size_t> inputPort;

    // init all paraments
    for (const Net *net : graphHandler->getAllNets()) {
        mSchaltung[net->getId()] = Logic::logicX;
        mNetMap[net->getId()] = false;
        if (!net->getInElement())
            inputPort.push_back(net->getId());
        else if (ElementType::Dff == net->getInElement()->getElementInfo()->getType())
            mRegDff[net->getId()] = Logic::logicX;
        if (!net->getOutElements().at(0)) 
            outputPort.push_back(net->getId());
    }

    // print the name of ports
    for (auto port : outputPort)
        std::cout << graphHandler->getNet(port)->getName() << "\t";
    std::cout << std::endl;

    // input dates per clock
    for (const std::vector<Logic> &timeStep : inputData) {
        unsigned long i = 0;

        for (auto iter = mNetMap.begin(); iter != mNetMap.end(); iter++)
            iter->second = false;

        // init input port
        for (const Logic &value : timeStep) {
            mSchaltung[inputPort[i]] = value;
            mNetMap[inputPort[i]] = true;
            i++;
        }

        // init Dff resgister
        for (auto it = mRegDff.begin(); it != mRegDff.end(); it++) {
            mSchaltung[graphHandler->getNet(it->first)->getId()] = it->second;
            mNetMap[it->first] = true;
        }

        // calcuate logic value of all nets and print
        for (std::vector<size_t>::iterator it = outputPort.begin(); it != outputPort.end(); it++) {
            Logic out = calLogicResult(*it);
            std::cout << out << ";\t";
        }

        std::cout << std::endl;

        // update all Dff resgsiters
        for (auto i = mRegDff.begin(); i != mRegDff.end(); i++){
            if ("CLOCK" == graphHandler->getNet(i->first)->getInElement()->getInNets()[0]->getName())
                i->second = calLogicResult(graphHandler->getNet(i->first)->getInElement()->getInNets()[1]->getId());
            else
                i->second = calLogicResult(graphHandler->getNet(i->first)->getInElement()->getInNets()[0]->getId());
        }
    }
}

Logic AnalysisEDA::AND(Logic a, Logic b) {
    if (a == Logic::logic0 || b == Logic::logic0)
        return Logic::logic0;
    else if (a == Logic::logicX || b == Logic::logicX)
        return Logic::logicX;
    else
        return Logic::logic1;
}

Logic AnalysisEDA::OR(Logic a, Logic b) {
    if (a == Logic::logic1 || b == Logic::logic1)
        return Logic::logic1;
    else if (a == Logic::logicX || b == Logic::logicX)
        return Logic::logicX;
    else
        return Logic::logic0;
}

Logic AnalysisEDA::NOT (Logic a)
{
    if (a == Logic::logic0)
        return Logic::logic1;
    else if (a == Logic::logic1)
        return Logic::logic0;
    else
        return Logic::logicX;
}

Logic AnalysisEDA::calLogicResult(size_t net) {
    if (true == mNetMap[net])
        return mSchaltung[net];
    else {
        const Element *ele = graphHandler->getNet(net)->getInElement();
        for (const Net *net : graphHandler->getNet(net)->getInElement()->getInNets())
            mSchaltung[net->getId()] = calLogicResult(net->getId());
        mNetMap[net] = true;

        switch (graphHandler->getNet(net)->getInElement()->getElementInfo()->getType()) {
            case ElementType::And:
                return AND(mSchaltung[ele->getInNets()[0]->getId()], mSchaltung[ele->getInNets()[1]->getId()]);
            case ElementType::Or:
                return OR(mSchaltung[ele->getInNets()[0]->getId()],mSchaltung[ele->getInNets()[1]->getId()]);
            case ElementType::Not:
                return NOT(mSchaltung[ele->getInNets()[0]->getId()]);
            default:
                break;
        }
    }
    return Logic::logicX;
}