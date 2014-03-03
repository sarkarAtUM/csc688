/*
 * Main.cpp
 *
 *  Created on: Feb 19, 2014
 *      Author: sam
 */

#include "../Framework.h"
#include <iostream>
#include <csignal>

void signalHandler(int signum)
{
  std::cout << "Interrupt signal (" << signum << ") received" << std::endl;
  // cleanup and close up stuff here
  // terminate program
  Graph::deleteInstance();
  exit(signum);
}

int main(int argc, char** argv)
{
  std::cout << "*** start ***" << std::endl;
  // register signal SIGINT and signal handler
  signal(SIGINT, signalHandler);
  Graph& graph = Graph::getInstance();
  graph.computeGraph();
  graph.topoSort();
  graph.stream();
  graph.graphOutputInit();
  for (;;)
    Graph::getInstance().graphOutputUpdate();
  Graph::deleteInstance();
  std::cout << "*** end ***" << std::endl;
  return 0;
}

