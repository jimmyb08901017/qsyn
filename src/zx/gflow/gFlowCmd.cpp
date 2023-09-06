/****************************************************************************
  FileName     [ gFlowCmd.cpp ]
  PackageName  [ gflow ]
  Synopsis     [ Define gflow package commands ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cstddef>
#include <string>

#include "./gFlow.hpp"
#include "cli/cli.hpp"
#include "zx/zxCmd.hpp"
#include "zx/zxGraphMgr.hpp"

using namespace std;
using namespace ArgParse;

extern ZXGraphMgr zxGraphMgr;

Command ZXGGFlowCmd();

bool initGFlowCmd() {
    if (!cli.registerCommand(ZXGGFlowCmd())) {
        cerr << "Registering \"gflow\" commands fails... exiting" << endl;
        return false;
    }
    return true;
}

Command ZXGGFlowCmd() {
    return {"zxggflow",
            [](ArgumentParser& parser) {
                parser.description("calculate and print the generalized flow of a ZXGraph");

                auto mutex = parser.addMutuallyExclusiveGroup().required(false);

                mutex.addArgument<bool>("-all")
                    .action(storeTrue)
                    .help("print both GFlow levels and correction sets");
                mutex.addArgument<bool>("-levels")
                    .action(storeTrue)
                    .help("print GFlow levels");
                mutex.addArgument<bool>("-corrections")
                    .action(storeTrue)
                    .help("print the correction set to each ZXVertex");
                mutex.addArgument<bool>("-summary")
                    .action(storeTrue)
                    .help("print basic information on the ZXGraph's GFlow");

                parser.addArgument<bool>("-extended")
                    .action(storeTrue)
                    .help("calculate the extended GFlow, i.e., allowing XY, YZ, XZ plane measurements");

                parser.addArgument<bool>("-independent-set")
                    .action(storeTrue)
                    .help("force each GFlow level to be an independent set");
            },
            [](ArgumentParser const& parser) {
                if (!zxGraphMgrNotEmpty()) return CmdExecResult::ERROR;
                GFlow gflow(zxGraphMgr.get());

                gflow.doExtendedGFlow(parser.get<bool>("-extended"));
                gflow.doIndependentLayers(parser.get<bool>("-independent-set"));

                gflow.calculate();

                if (parser.parsed("-all")) {
                    gflow.print();
                } else if (parser.parsed("-levels")) {
                    gflow.printLevels();
                } else if (parser.parsed("-corrections")) {
                    gflow.printXCorrectionSets();
                }

                gflow.printSummary();
                if (!gflow.isValid()) gflow.printFailedVertices();

                return CmdExecResult::DONE;
            }};
}