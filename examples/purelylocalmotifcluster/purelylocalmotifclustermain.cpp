// localmotifclustermain.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "purelylocalmotifcluster.h"

int main(int argc, char* argv[]) {
  printf("{\n");
  printf("\"Run Info\": \"\n");
  Env = TEnv(argc, argv, TNotify::StdNotify);
  Env.PrepArgs(TStr::Fmt("Purely Local motif clustering. build: %s, %s. Time: %s",
       __TIME__, __DATE__, TExeTm::GetCurTm()));  
  TExeTm ExeTm;  
  Try

  const bool IsDirected = 
    Env.GetIfArgPrefixBool("-d:", false, "Directed graph?");

  ProcessedGraph graph_p;
  if (IsDirected) {
    const TStr graph_filename =
      Env.GetIfArgPrefixStr("-i:", "C-elegans-frontal.txt", "Input graph file");
    const TStr motif =
      Env.GetIfArgPrefixStr("-m:", "triad", "Motif type");
    MotifType mt = ParseMotifType(motif, IsDirected);
    PNGraph graph;
    if (graph_filename.GetFExt().GetLc() == ".ngraph") {
      TFIn FIn(graph_filename);
      graph = TNGraph::Load(FIn);
    } else if (graph_filename.GetFExt().GetLc() == ".ungraph") {
      TExcept::Throw("Warning: input graph is an undirected graph!!");
    } else {
      graph = TSnap::LoadEdgeList<PNGraph>(graph_filename, 0, 1);
    }
    TSnap::DelSelfEdges(graph);
    graph_p = ProcessedGraph(graph, mt);

  } else {

    const TStr graph_filename =
      Env.GetIfArgPrefixStr("-i:", "C-elegans-frontal.txt", "Input graph file");
    const TStr motif =
      Env.GetIfArgPrefixStr("-m:", "clique3", "Motif type");
    MotifType mt = ParseMotifType(motif, IsDirected);
    PUNGraph graph;
    if (graph_filename.GetFExt().GetLc() == ".ungraph") {
      TFIn FIn(graph_filename);
      graph = TUNGraph::Load(FIn);
    } else if (graph_filename.GetFExt().GetLc() == ".ngraph") {
      TExcept::Throw("Warning: input graph is a directed graph!!");
    } else {
      graph = TSnap::LoadEdgeList<PUNGraph>(graph_filename, 0, 1);
    }
    TSnap::DelSelfEdges(graph);
    graph_p = ProcessedGraph(graph, mt);
  }


  const TVec<TInt> seeds =
    Env.GetIfArgPrefixIntV("-s:", "Seeds");
  const TFlt alpha =
    Env.GetIfArgPrefixFlt("-a:", 0.98, "alpha");
  const TFlt eps =
    Env.GetIfArgPrefixFlt("-e:", 0.0001, "eps");

  printf("\"\n");
  printf(",\n");

  printf("\"Number of Seeds\": %d\n", seeds.Len());
  printf(",\n");
  for (int i = 0; i < seeds.Len(); i++) {
    int seed = seeds[i];
    printf("\"Seed\": %d \n", int(seed));
    printf(",\n");
    MAPPR mappr;
    graph_p.printTotalVolume();
    printf(",\n");
    mappr.computeAPPR(graph_p, seed, alpha, eps / graph_p.getTotalVolume() * graph_p.getTransformedGraph()->GetNodes());
    mappr.sweepAPPR(-1);
    // mappr.printProfile();
    printf("\"Size of Found Cluster\": %d.\n", mappr.getCluster().Len());
    printf(",\n");
    printf("\"Nodes of Cluster\": [");
    printf("%d", int(mappr.getCluster()[0]));
    for (int i = 1; i < mappr.getCluster().Len(); i++) {
      printf(", %d", int(mappr.getCluster()[i]));
    }
    printf("]\n");
    printf(",\n");
    
    printf("\"Weights Computed\": %d/%d\n", graph_p.getWeights().Len(), graph_p.numNodes);
    printf(",\n");
  }

  Catch
  printf("\"Run Time\": %s \n", ExeTm.GetTmStr());
  printf("}\n");
  return 0;
}
