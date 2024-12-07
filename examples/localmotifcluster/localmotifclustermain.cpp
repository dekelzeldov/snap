// localmotifclustermain.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "localmotifcluster.h"

int main(int argc, char* argv[]) {
  printf("{\n");
  printf("\"Version\": \"Local\",\n");
  Env = TEnv(argc, argv, TNotify::StdNotify);
  Env.PrepArgs(TStr::Fmt("Local motif clustering. build: %s, %s. Time: %s",
       __TIME__, __DATE__, TExeTm::GetCurTm()));  
  TExeTm ExeTm;  
  Try

  const bool IsDirected = 
    Env.GetIfArgPrefixBool("-d:", false, "Directed graph?");
  printf("\"Directed\": ");
  IsDirected ? printf("true") : printf("false") ;
  printf(",\n");

  ProcessedGraph graph_p;
  if (IsDirected) {
    const TStr graph_filename =
      Env.GetIfArgPrefixStr("-i:", "C-elegans-frontal.txt", "Input graph file");
    const TStr motif =
      Env.GetIfArgPrefixStr("-m:", "triad", "Motif type");
    MotifType mt = ParseMotifType(motif, IsDirected);
    printf("\"Motif\": \"");
    printf(motif.CStr());
    printf("\",\n");
    TExeTm ReadGraphTm;
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
    printf("\"Read Graph Time (seconds)\": %.2f, \n", ReadGraphTm.GetSecs());
    graph_p = ProcessedGraph(graph, mt);

  } else {

    const TStr graph_filename =
      Env.GetIfArgPrefixStr("-i:", "C-elegans-frontal.txt", "Input graph file");
    const TStr motif =
      Env.GetIfArgPrefixStr("-m:", "clique3", "Motif type");
    MotifType mt = ParseMotifType(motif, IsDirected);
    printf("\"Motif\": \"");
    printf(motif.CStr());
    printf("\",\n");
    TExeTm ReadGraphTm;
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
    printf("\"Read Graph Time (seconds)\": %.2f, \n", ReadGraphTm.GetSecs());
    graph_p = ProcessedGraph(graph, mt);
  }


  const TVec<TInt> seeds =
    Env.GetIfArgPrefixIntV("-s:", "Seeds");
  const TFlt alpha =
    Env.GetIfArgPrefixFlt("-a:", 0.98, "alpha");
  const TFlt eps =
    Env.GetIfArgPrefixFlt("-e:", 0.0001, "eps");
  const TFlt just_volume =
    Env.GetIfArgPrefixBool("-v:", false, "just volume");
  printf("\"Just Volume\": ");
  just_volume ? printf("true") : printf("false") ;
  printf(",\n");

  printf("\"Number of Seeds\": %d, \n", seeds.Len());
  for (int i = 0; i < seeds.Len(); i++) {
    int seed = seeds[i];
    printf("\"Seed\": %d, \n", int(seed));
    MAPPR mappr;
    printf("\"Total Volume\": %.2f, \n", graph_p.getTotalVolume());
    printf("\"Number of Nodes\": %.2f, \n", graph_p.getTransformedGraph()->GetNodes());
    if (!just_volume){
      TExeTm APPRTm; 
      mappr.computeAPPR(graph_p, seed, alpha, eps / graph_p.getTotalVolume() * graph_p.getTransformedGraph()->GetNodes());
      mappr.sweepAPPR(-1);
      //mappr.printProfile();
      printf("\"APPR Time (seconds)\": %.2f, \n", APPRTm.GetSecs());
      printf("\"Found Cluster Size\": %d, \n", mappr.getCluster().Len());
      printf("\"Found Cluster\": [");
      printf("%d", int(mappr.getCluster()[0]));
      for (int i = 1; i < mappr.getCluster().Len(); i++) {
        printf(", %d", int(mappr.getCluster()[i]));
      }
      printf("], \n");
    }

  printf("\"Weights Computed\": \"N/A\",\n");
  }

  Catch
  printf("\"Run Time (seconds)\": %.2f \n", ExeTm.GetSecs());
  printf("}\n");
  return 0;
}
