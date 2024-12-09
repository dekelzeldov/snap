#include "purelylocalmotifcluster.h"
#include "stdafx.h"


// void printVec(const TIntV& vec) {
//   printf("[");
//   for (int i = 0; i < vec.Len(); i ++) {
//     int val = vec[i];
//     printf("%d ", val);
//   }
//   printf("]\n");
// }




/*
Section: Motif parsing and printing
*/
// Given a string representation of a motif name, parse it to a MotifType.
MotifType ParseMotifType(const TStr& motif, const bool& IsDirected) {
  TStr motif_lc = motif.GetLc();
  if (IsDirected) {
    if      (motif_lc == "m1")          { return M1; }
    else if (motif_lc == "m2")          { return M2; }
    else if (motif_lc == "m3")          { return M3; }
    else if (motif_lc == "m4")          { return M4; }
    else if (motif_lc == "m5")          { return M5; }
    else if (motif_lc == "m6")          { return M6; }
    else if (motif_lc == "m7")          { return M7; }
    else if (motif_lc == "triad")          { return triad; }
    else if (motif_lc == "cycle")          { return cycle; }
    else if (motif_lc == "ffloop")          { return FFLoop; }
    else if (motif_lc == "unide")          { return UniDE; }
    else if (motif_lc == "bide")          { return BiDE; }
    else if (motif_lc == "de")          { return DE; }
    else if (motif_lc == "edge")          { return DE_any; }
    else { 
      TExcept::Throw("Unknown motif for directed graph!"); 
    }
  } else {
    if      (motif_lc == "uedge")     { return UEdge; }
    else if (motif_lc == "clique3")     { return clique3; }
    else if (motif_lc == "clique4")     { return clique4; }
    else if (motif_lc == "clique5")     { return clique5; }
    else if (motif_lc == "clique6")     { return clique6; }
    else if (motif_lc == "clique7")     { return clique7; }
    else if (motif_lc == "clique8")     { return clique8; }
    else if (motif_lc == "clique9")     { return clique9; }
    else { TExcept::Throw("Unknown motif for undirected graph!"); }
  }
  TExcept::Throw("Inappropriate input!"); 
  return UEdge;
}

// Print motif type, mainly for debugging use
void printMotifType(const MotifType& type) {
  switch (type) {
    case M1:      printf("M1\n"); break;
    case M2:      printf("M2\n"); break;
    case M3:      printf("M3\n"); break;
    case M4:      printf("M4\n"); break;
    case M5:      printf("M5\n"); break;
    case M6:      printf("M6\n"); break;
    case M7:      printf("M7\n"); break;
    case triad:   printf("triad\n"); break;
    case cycle:   printf("cycle\n"); break;
    case FFLoop:  printf("FFLoop\n"); break;
    case UniDE:   printf("UniDE\n"); break;
    case BiDE:    printf("BiDE\n"); break;
    case DE:      printf("DE\n"); break;
    case DE_any:  printf("DE_any\n"); break;

    case UEdge:       printf("UEdge\n"); break;
    case clique3:     printf("clique3\n"); break;
    case clique4:     printf("clique4\n"); break;
    case clique5:     printf("clique5\n"); break;
    case clique6:     printf("clique6\n"); break;
    case clique7:     printf("clique7\n"); break;
    case clique8:     printf("clique8\n"); break;
    case clique9:     printf("clique9\n"); break;

    default:
      TExcept::Throw("Unknown motif type!");
  }
}

// Get the clique size from undirected motif type
int getCliqueSize(const MotifType& type) {
  switch (type) {
    case UEdge:       return 2;
    case clique3:     return 3;
    case clique4:     return 4;
    case clique5:     return 5;
    case clique6:     return 6;
    case clique7:     return 7;
    case clique8:     return 8;
    case clique9:     return 9;
    default: {
      TExcept::Throw("Unknown motif for undirected graph!"); 
      return -1;
    }         
  }
}



/*
Section: Preprocessing unweighted graphs, including:
1. counting motif on each edge
2. obtain a transformed weighted graph.
*/

// Initializing for undirected graph input.
ProcessedGraph::ProcessedGraph(PUNGraph graph, MotifType mt){
  Graph_org = graph;
  this->mt = mt;
  Directed = false;
  numNodes = Graph_org->GetNodes();
  prepWeights_undir();
}


// This function will return true if degree of nodeID1 is higher than nodeID2.
// If the degrees equal, it will return true if nodeID1 > nodeID2.
// Used in ChibaNishizeki's clique enumeration method
bool higherDeg(PUNGraph& G, TUNGraph::TNodeI& NI1, int nodeID2) {
  TUNGraph::TNodeI NI2 = G->GetNI(nodeID2);
  if (NI1.GetOutDeg() > NI2.GetOutDeg()) {
    return true;
  } else if (NI1.GetOutDeg() == NI2.GetOutDeg() && NI1.GetId() > nodeID2) {
    return true;
  } else {
    return false;
  }
}
bool higherDeg(PUNGraph& G, int nodeID1, int nodeID2) {
  TUNGraph::TNodeI NI1 = G->GetNI(nodeID1);
  return higherDeg(G, NI1, nodeID2);
}

// This function counts the undirected graph motif (clique) instances on each edge of a node.
void ProcessedGraph::countClique(int nodeID, int KSize, TIntV& PrevNodes) {
  // Each edge means a (level+2)-clique in the original graph!
  if (Computed[nodeID]) {
    return;
  }

  TUNGraph::TNodeI NI = Graph_org->GetNI(nodeID);
  int degHere = NI.GetOutDeg();

  if (!Counts.IsKey(nodeID)) {
    for (int e = 0; e < degHere; e++) {
      int nbrID = NI.GetOutNId(e);
      Counts(nodeID)(nbrID) = TIntV(KSize - 2);
    }
  }

  // Go to the next level
  PrevNodes[0] = nodeID;
  TIntV neighborsID;
  for (int e = 0; e < NI.GetOutDeg(); e++) {
    int nbrID = NI.GetOutNId(e);
    if (!Counts.IsKey(nbrID)) {
      TUNGraph::TNodeI nbrNI = Graph_org->GetNI(nbrID);
      int nbrDeg = nbrNI.GetOutDeg();
      for (int e = 0; e < nbrDeg; e++) {
        int nbrnbrID = nbrNI.GetOutNId(e);
        Counts(nbrID)(nbrnbrID) = TIntV(KSize - 2);
      }
    }
    if (!Computed[nbrID]) {
      Dependants(nodeID).AddKey(nbrID);
    } 
    if (higherDeg(Graph_org, nodeID, nbrID)) {
        neighborsID.Add(nbrID);
    }
  }
  PUNGraph subGraph = TSnap::GetSubGraph(Graph_org, neighborsID);
  countClique(subGraph, KSize, PrevNodes, 1);
  Computed[nodeID] = true;
}

// This function counts the undirected graph motif (clique) instances on each edge.
// It uses recursive method for clique enumeration proposed by Chiba and Nishizeki (SIAM J. Comput. 1985).
// Input {int KSize} denotes the size of the clique we are to enumerate in the current graph {PUNGraph& G}
// Input {TIntV& PrevNodes} denotes a set of nodes that are directed connected to any node in the current graph G
//    and {int level = PrevNodes.Len()} is the number of PreNodes. Therefore, any k-clique in G corresponds to 
//    a (k+level)-clique after all nodes in PrevNodes are added in the current graph G.
void ProcessedGraph::countClique(PUNGraph& G, int KSize, TIntV& PrevNodes, int level) {
  // Each edge means a (level+2)-clique in the original graph!
  if (level >= KSize - 1) {
    throw "exception!!";
  }
  if (level >= 1) {
    for (TUNGraph::TEdgeI EI = G->BegEI(); EI < G->EndEI(); EI ++ ) {
      int SrcNId = EI.GetSrcNId();
      int DstNId = EI.GetDstNId();
      Counts(SrcNId)(DstNId)[level-1] ++;
      Counts(DstNId)(SrcNId)[level-1] ++;
    }
  }
  for (TUNGraph::TNodeI NI = G->BegNI(); NI < G->EndNI(); NI ++ ) {
    int NodeId = NI.GetId();
    int degHere = NI.GetOutDeg();
    for (int i = 0; i < level; i ++) {
      Counts(PrevNodes[i])(NodeId)[level-1] += degHere;
      Counts(NodeId)(PrevNodes[i])[level-1] += degHere;
    }

    if (level == KSize - 2) {
      continue;
    }
    // Go to the next level
    PrevNodes[level] = NodeId;
    TIntV neighborsID;
    for (int e = 0; e < NI.GetOutDeg(); e++) {
      int nbrID = NI.GetOutNId(e);
      if (!Computed[nbrID]) {
        Dependants(PrevNodes[0]).AddKey(nbrID);
      }
      if (!Counts.IsKey(nbrID)) {
        TUNGraph::TNodeI nbrNI = G->GetNI(nbrID);
        int nbrDeg = nbrNI.GetOutDeg();
        for (int e = 0; e < nbrDeg; e++) {
          int nbrnbrID = nbrNI.GetOutNId(e);
          Counts(nbrID)(nbrnbrID) = TIntV(KSize - 2);
        }
      }
      if (higherDeg(G, NodeId, nbrID)) {
          neighborsID.Add(nbrID);
      }
    }
    PUNGraph subGraph = TSnap::GetSubGraph(G, neighborsID);
    int numEdges = subGraph->GetEdges();
    for (int i = 0; i <= level; i ++) {
      for (int j = i + 1; j <= level; j ++) {
        Counts(PrevNodes[i])(PrevNodes[j])[level] += numEdges;
        Counts(PrevNodes[j])(PrevNodes[i])[level] += numEdges;
      }
    }
    countClique(subGraph, KSize, PrevNodes, level + 1);
  }
}


// Functions for undirected graph that
//  1) counts motifs on each edge of the given nodes
//  2) assign weights acoordingly
void ProcessedGraph::assignWeights_undir(int nodeID) {
  TUNGraph::TNodeI NI = Graph_org->GetNI(nodeID);
  Weights(nodeID) = NodeWeightVH(NI.GetOutDeg());
  int KSize = getCliqueSize(mt);
  if (KSize == 2) {
    // Don't need to count, assign weights directly!
    for (int e = 0; e < NI.GetOutDeg(); e++) {
      Weights(nodeID)(NI.GetOutNId(e)) = 1;
    }
    Weights(nodeID)(nodeID) = NI.GetOutDeg();
    TotalVolLB += NI.GetOutDeg();
  } else { 
    // KSize > 2
    TIntV PrevNodes(KSize - 2);
    countClique(nodeID, KSize, PrevNodes);
    while (Dependants(nodeID).BegI() < Dependants(nodeID).EndI()) {
      int key = Dependants(nodeID).BegI().GetKey();
      countClique(key, KSize, PrevNodes);
      Dependants(nodeID).DelKey(key);
    }
    Dependants.DelKey(nodeID);

    // Now we assign weights!
    float deg_w = 0;
    for (int e = 0; e < NI.GetOutDeg(); e++) {
      int NbrId = NI.GetOutNId(e);
      int MtfCntHere = Counts(nodeID)(NbrId)[KSize-3];
      if (MtfCntHere) {
        Weights(nodeID)(NbrId) = MtfCntHere;
        deg_w += MtfCntHere;
      } else {
        Graph_trans->DelEdge(nodeID, NbrId);
      }
    }
    Weights(nodeID)(nodeID) = deg_w;
    TotalVolLB += deg_w;
  }
}

// Functions for undirected graph that
//  1) counts motifs on each pair of nodes
//  2) assign weights
//  3) obtain the transformed graph
void ProcessedGraph::prepWeights_undir() {
  Weights = WeightVH(Graph_org->GetMxNId());
  Graph_trans = TSnap::ConvertGraph<PUNGraph>(Graph_org);
  TotalVolLB = 0;
  TotalVol = -1;
  int KSize = getCliqueSize(mt);
  if (KSize == 2) {
    TotalVol = 2 * Graph_org->GetEdges();
  } else { 
    if (Counts.Len() == 0 || Counts.BegI()->Dat.Len() == 0 || Counts.BegI()->Dat.BegI()->Dat.Len() < KSize - 2) {
      // If the KSize-clique has not been counted yet, then we count.
      Counts = CountVH(Graph_org->GetMxNId());
      Dependants = DependVH(Graph_org->GetMxNId());
      Computed = TVec<bool>(Graph_org->GetMxNId());
      Computed.PutAll(false);
    }
  }
}


// Initializing for directed graph input.
ProcessedGraph::ProcessedGraph(PNGraph graph, MotifType mt){
  Graph_org_dir = graph;
  Graph_org = TSnap::ConvertGraph<PUNGraph>(graph);
  this->mt = mt;
  Directed = true;
  numNodes = Graph_org->GetNodes();
  prepWeights_dir();
}
 
// Check the edge information between nodeID and nbrID;
// It assume that nbrID is a neighbor of nodeID, either in-neighbor or out-neighbor;
// It will return
//   0: if nodeID <-> nbrID
//   1: if nodeID --> nbrID
//   2: if nodeID <-- nbrID
int checkEdge(PNGraph& G, long nodeID, long nbrID) {
  if (G->IsEdge(nodeID, nbrID)) {
    if (G->IsEdge(nbrID, nodeID)) {
      return 0;
    } else {
      return 1;
    }
  } else {
    return 2;
  }
}

// To check the motif types of the triad (nodeID, srcNId, dstNId).
// We have already known that (srcNId -> dstNId) is an edge in G
int checkTriadMotif(PNGraph& G, long nodeID, long srcNId, long dstNId) {
  int motifType = 0;
  if (G->IsEdge(dstNId, srcNId)) {
    switch (checkEdge(G, nodeID, dstNId)) {
      case 0: {
        switch (checkEdge(G, nodeID, srcNId)) {
          case 0: {
            motifType = 4;
            break;
          }
          case 1: {
            motifType = 3;
            break;
          }
          case 2: {
            motifType = 3;
          }
        }
        break;
      }
      case 1: {
        switch (checkEdge(G, nodeID, srcNId)) {
          case 0: {
            motifType = 3;
            break;
          }
          case 1: {
            motifType = 6;
            break;
          }
          case 2: {
            motifType = 2;
          }
        }
        break;
      }
      case 2: {
        switch (checkEdge(G, nodeID, srcNId)) {
          case 0: {
            motifType = 3;
            break;
          }
          case 1: {
            motifType = 2;
            break;
          }
          case 2: {
            motifType = 7;
          }
        }
      }
    }
  } else {
    switch (checkEdge(G, nodeID, dstNId)) {
      case 0: {
        switch (checkEdge(G, nodeID, srcNId)) {
          case 0: {
            motifType = 3;
            break;
          }
          case 1: {
            motifType = 2;
            break;
          }
          case 2: {
            motifType = 6;
          }
        }
        break;
      }
      case 1: {
        switch (checkEdge(G, nodeID, srcNId)) {
          case 0: {
            motifType = 7;
            break;
          }
          case 1: {
            motifType = 5;
            break;
          }
          case 2: {
            motifType = 5;
          }
        }
        break;
      }
      case 2: {
        switch (checkEdge(G, nodeID, srcNId)) {
          case 0: {
            motifType = 2;
            break;
          }
          case 1: {
            motifType = 1;
            break;
          }
          case 2: {
            motifType = 5;
          }
        }
      }
    }
  }
  return motifType;
}


void ProcessedGraph::setMtfInclude() {
  switch (mt) {
    case UniDE :      {MtfInclude.Add(0); break;}
    case BiDE :       MtfInclude.Add(1); break;
    case DE :         MtfInclude.Add(0); MtfInclude.Add(1); MtfInclude.Add(1); break;
    case DE_any :     MtfInclude.Add(0); MtfInclude.Add(1); break;

    case M1 :         MtfInclude.Add(2); break;
    case M2 :         MtfInclude.Add(3); break;
    case M3 :         MtfInclude.Add(4); break;
    case M4 :         MtfInclude.Add(5); break;
    case M5 :         MtfInclude.Add(6); break;
    case M6 :         MtfInclude.Add(7); break;
    case M7 :         MtfInclude.Add(8); break;
    case triad : {
      MtfInclude.Add(2); 
      MtfInclude.Add(3); 
      MtfInclude.Add(4); 
      MtfInclude.Add(5); 
      MtfInclude.Add(6); 
      MtfInclude.Add(7); 
      MtfInclude.Add(8); 
      break;
    }
    case cycle : {
      MtfInclude.Add(2); 
      MtfInclude.Add(3); 
      MtfInclude.Add(4); 
      MtfInclude.Add(5); 
      MtfInclude.Add(5); 
      break;
    }
    case FFLoop : {
      MtfInclude.Add(6); 
      MtfInclude.Add(7); 
      MtfInclude.Add(7); 
      MtfInclude.Add(8); 
      MtfInclude.Add(8); 
      break;
    default:
      TExcept::Throw("Unknown motif type!");
    }    
  }
}


// This function will return true if the out-degree of nodeID1 is higher than nodeID2.
// If the degrees equal, it will return true if nodeID1 > nodeID2.
// Used in ChibaNishizeki's clique enumeration method
bool higherDeg(PNGraph& G, TNGraph::TNodeI& NI1, int nodeID2) {
  TNGraph::TNodeI NI2 = G->GetNI(nodeID2);
  if (NI1.GetOutDeg() > NI2.GetOutDeg()) {
    return true;
  } else if (NI1.GetOutDeg() == NI2.GetOutDeg() && NI1.GetId() > nodeID2) {
    return true;
  } else {
    return false;
  }
}
bool higherDeg(PNGraph& G, int nodeID1, int nodeID2) {
  TNGraph::TNodeI NI1 = G->GetNI(nodeID1);
  return higherDeg(G, NI1, nodeID2);
}

// To count the directed triangle motifs of a node
void ProcessedGraph::countDirTriadMotif(int nodeID) {
  int numBasicDirMtf = 9;  

  if (Computed[nodeID]) {
    return;
  }

  TIntV neighborsID;
  TNGraph::TNodeI NI = Graph_org_dir->GetNI(nodeID);

  if (!Counts.IsKey(nodeID)) {
    for (int e = 0; e < NI.GetDeg(); e++) {
      int nbrID = NI.GetNbrNId(e);
      Counts(nodeID)(nbrID) = TIntV(numBasicDirMtf);
    }
  }

  for (int e = 0; e < NI.GetOutDeg(); e++) {
    int nbrID = NI.GetOutNId(e);
    if (!Computed[nbrID]) {
      Dependants(nodeID).AddKey(nbrID);
    }
    if (!Counts.IsKey(nbrID)) {
      TNGraph::TNodeI nbrNI = Graph_org_dir->GetNI(nbrID);
      for (int e = 0; e < nbrNI.GetDeg(); e++) {
        int nbrnbrID = nbrNI.GetNbrNId(e);
        Counts(nbrID)(nbrnbrID) = TIntV(numBasicDirMtf);
      }
    }
    if (higherDeg(Graph_org_dir, nodeID, nbrID)) {
      neighborsID.Add(nbrID);
      Counts(nodeID)(nbrID)[0] ++;
      Counts(nbrID)(nodeID)[0] ++;
    }
  }
  for (int e = 0; e < NI.GetInDeg(); e++) {
    int nbrID = NI.GetInNId(e);
    if (!Computed[nbrID]) {
      Dependants(nodeID).AddKey(nbrID);
    }
    if (!Counts.IsKey(nbrID)) {
      TNGraph::TNodeI nbrNI = Graph_org_dir->GetNI(nbrID);
      for (int e = 0; e < nbrNI.GetDeg(); e++) {
        int nbrnbrID = nbrNI.GetNbrNId(e);
        Counts(nbrID)(nbrnbrID) = TIntV(numBasicDirMtf);
      }
    }
    if (higherDeg(Graph_org_dir, nodeID, nbrID)) {
      if (Graph_org_dir->IsEdge(nodeID, nbrID)) {
        Counts(nodeID)(nbrID)[0] --;
        Counts(nbrID)(nodeID)[0] --;
        Counts(nodeID)(nbrID)[1] ++;
        Counts(nbrID)(nodeID)[1] ++;
      } else {
        neighborsID.Add(nbrID);
        Counts(nodeID)(nbrID)[0] ++;
        Counts(nbrID)(nodeID)[0] ++;
      }
    }
  }
  PNGraph subGraph = TSnap::GetSubGraph(Graph_org_dir, neighborsID);
  for (TNGraph::TEdgeI EI = subGraph->BegEI(); EI < subGraph->EndEI(); EI ++ ) { 
    int srcNId = EI.GetSrcNId();
    int dstNId = EI.GetDstNId();
    int MotifNumber = 0;
    if (srcNId > dstNId || !subGraph->IsEdge(dstNId, srcNId)) {
      MotifNumber = checkTriadMotif(Graph_org_dir, nodeID, srcNId, dstNId);
      MotifNumber ++;
      Counts(nodeID)(srcNId)[MotifNumber] ++;
      Counts(srcNId)(nodeID)[MotifNumber] ++;
      Counts(nodeID)(dstNId)[MotifNumber] ++;
      Counts(dstNId)(nodeID)[MotifNumber] ++;
      Counts(srcNId)(dstNId)[MotifNumber] ++;
      Counts(dstNId)(srcNId)[MotifNumber] ++;
    }
  }
  Computed[nodeID] = true;
}


// Functions for directed graph that
//  1) counts motifs on each edge of a nodes
//  2) assign weights
//  3) obtain the transformed graph
void ProcessedGraph::assignWeights_dir(int nodeID) {
  countDirTriadMotif(nodeID);
  while (Dependants(nodeID).BegI() < Dependants(nodeID).EndI()) {
    int key = Dependants(nodeID).BegI().GetKey();
    countDirTriadMotif(key);
    Dependants(nodeID).DelKey(key);
  }

  TUNGraph::TNodeI NI = Graph_org->GetNI(nodeID);
  float deg_w = 0;
  for (int e = 0; e < NI.GetOutDeg(); e++) {
    int NbrId = NI.GetOutNId(e);
    TIntV& CountHere = Counts(nodeID)(NbrId);
    int WeightHere = 0;
    for (int i = 0; i < MtfInclude.Len(); i ++) {
      WeightHere += CountHere[MtfInclude[i]];
    }
    if (WeightHere) {
      Weights(nodeID)(NbrId) = WeightHere; 
      deg_w += WeightHere;
    } else {
      Graph_trans->DelEdge(nodeID, NbrId);
    }
  }
  Weights(nodeID)(nodeID) = deg_w;
  TotalVolLB += deg_w;
}

// Functions for directed graph to prepare the weights and the transformed graph
void ProcessedGraph::prepWeights_dir() {
  setMtfInclude();
  Counts = CountVH(Graph_org->GetMxNId());
  Dependants = DependVH(Graph_org->GetMxNId());
  Computed = TVec<bool>(Graph_org->GetMxNId());
  Computed.PutAll(false);
  Graph_trans = TSnap::ConvertGraph<PUNGraph>(Graph_org);
  Weights = WeightVH(Graph_org->GetMxNId());
  TotalVolLB = 0;
  TotalVol = -1;
}

NodeWeightVH &ProcessedGraph::getNodeWeights(int nodeID) {
	if (!Weights.IsKey(nodeID)) { 
    assignWeights(nodeID);
  }
  return Weights.GetDat(nodeID);
}

void ProcessedGraph::estimateTotalVolume() {
  TExeTm EstTm;  
	int sample_size = 2401; 
	if (numNodes < sample_size)
	{
		for (TUNGraph::TNodeI NI = Graph_org->BegNI(); NI < Graph_org->EndNI(); NI++)
		{
			assignWeights(NI.GetId());
		}
		TotalVol = TotalVolLB;
	} else {

    int sampleVol = 0;
    float factore = numNodes/sample_size;

    THashSet<TInt> elems;
    int k = sample_size;
    if (factore > 0.5) {
      k = numNodes - k;
    }

    while (elems.Len() < k) {
      TInt n = Graph_org->GetRndNId();
      if (factore <= 0.5 and !elems.IsKey(n)) {
        assignWeights(n);
        sampleVol += Weights(n)(n);
      }
      elems.AddKey(n);
    }
    
    if (factore > 0.5) {
      int i = 0;
      for (TUNGraph::TNodeI NI = Graph_org->BegNI(); NI < Graph_org->EndNI(); NI++)
      {
        if (!elems.IsKey(i)) {
          int n = NI.GetId();
          assignWeights(n);
          sampleVol += Weights(n)(n);
        }
        i ++;
      }
    }

    TotalVolEst = sampleVol * factore;
  }
  printf("\"Vol Est. Time (seconds)\": %.2f,\n", EstTm.GetSecs());
}

void ProcessedGraph::printCounts(int NodeId) {  
  TUNGraph::TNodeI NI = Graph_org->GetNI(NodeId);
  for (int e = 0; e < NI.GetOutDeg(); e++) {
    int NbrId = NI.GetOutNId(e);
    TIntV& CountThisEdge = Counts(NodeId)(NbrId);
    printf("(%d, %d): ", NodeId, NbrId);
    for (int i = 0; i < CountThisEdge.Len(); i ++) {
      int output = CountThisEdge[i];
      printf("%d ", output);
    }
    printf("\n");
  }
}

void ProcessedGraph::printCounts() {  
  for (TUNGraph::TNodeI NI = Graph_org->BegNI(); NI < Graph_org->EndNI(); NI ++ ) {
    int NodeId = NI.GetId();
    for (int e = 0; e < NI.GetOutDeg(); e++) {
      int NbrId = NI.GetOutNId(e);
      TIntV& CountThisEdge = Counts(NodeId)(NbrId);
      printf("(%d, %d): ", NodeId, NbrId);
      for (int i = 0; i < CountThisEdge.Len(); i ++) {
        int output = CountThisEdge[i];
        printf("%d ", output);
      }
      printf("\n");
    }
  }
}

void ProcessedGraph::printWeights() {  
  for (TUNGraph::TNodeI NI = Graph_org->BegNI(); NI < Graph_org->EndNI(); NI ++ ) {
    int NodeId = NI.GetId();
    if (Weights.IsKey(NodeId)) {
      for (int e = 0; e < NI.GetOutDeg(); e++) {
        int NbrId = NI.GetOutNId(e);
        printf("(%d, %d): ", NodeId, NbrId);
        if (Weights.GetDat(NodeId).IsKey(NbrId)) {
          float weight = Weights.GetDat(NodeId).GetDat(NbrId);
          printf("%.2f ", weight);
        } else {
          printf("Not a key in Weights");
        }
        printf("\n");
      }
      float d_w = Weights.GetDat(NodeId).GetDat(NodeId);
      printf("\td_w(%d) = %.2f.\n", NodeId, d_w);
    } else {
      printf("Node %d is not a key in Weights.\n", NodeId);
    }
  }
  printf("Total Volume = %.2f. \n", TotalVol);
  Graph_trans->Dump();
}

void ProcessedGraph::printWeights(int NodeId) {  
    TUNGraph::TNodeI NI = Graph_org->GetNI(NodeId);
    for (int e = 0; e < NI.GetOutDeg(); e++) {
      int NbrId = NI.GetOutNId(e);
      printf("(%d, %d): ", NodeId, NbrId);
      if (Weights(NodeId).IsKey(NbrId)) {
        float weight = Weights(NodeId)(NbrId);
        printf("%.2f ", NbrId, weight);
      } else {
        printf("Not a key in Weights");
      }
      printf("\n");
    }
    float d_w = Weights(NodeId)(NodeId);
    printf("\td_w(%d) = %.2f.\n", NodeId, d_w);
}

bool ProcessedGraph::totalVolume_lte(float value) {
  if (TotalVol != -1) {
    return (TotalVol <= value);
  }
  if (TotalVolLB > value) {
    return false;
  }
  int numNodes = Graph_org->GetMxNId();
  while (TotalVolLB <= value and TotalVol == -1) {
    while (Dependants.BegI() < Dependants.EndI()) {
      TInt NIID = Dependants.BegI().GetKey();
      while (Dependants(NIID).BegI() < Dependants(NIID).EndI()) {
        if (TotalVolLB > value) {
          return false;
        }
        int key = Dependants(NIID).BegI().GetKey();
        if (!Weights.IsKey(key)) {
          assignWeights(key);
        }
        Dependants(NIID).DelKey(key);
      }
      Dependants.DelKey(NIID);
      if (TotalVolLB > value) {
        return false;
      }
    }
    while (Weights.Len()<numNodes and ToCompute < numNodes and Weights.IsKey(ToCompute)) {
      ToCompute ++;
    }
    if (ToCompute == numNodes) {
      TotalVol = TotalVolLB;
      return (TotalVol <= value);
    }
    assignWeights(ToCompute);
    ToCompute ++;
  }
  if (TotalVolLB > value) {
    
    return false;
  }
  return (TotalVol <= value);
}

void ProcessedGraph::printTotalVolume() const {
  if (TotalVol != -1) {
    printf("\"Total Volume\": %.2f, \n", TotalVol);
  } else {
    printf("\"Est. Total Volume\": %.2f, \n", TotalVolEst);
  }
}

float ProcessedGraph::getTotalVolume() const {
  if (TotalVol != -1) {
    return TotalVol;
  } else {
    return TotalVolEst;
  }
}




/*
Section: MAPPR main part, including:
1. compute the APPR vector on the weighted transformed graph
2. obtain the cluster by sweeping the NCP profile
*/
MAPPR::MAPPR() {
  NumPushs = 0;
  appr_norm = 0;
  SizeGlobalMin = 0;
  SizeFirstLocalMin = -1;
}

// Function to compute the APPR vector on the weighted transformed graph {ProcessedGraph graph_p} 
//    with seed {int SeedNodeId} alpha = {float alpha}, and epsilon = {float eps}.
// It will also compute the NCP as well as with {SizeGlobalMin} and {SizeFirstLocalMin}, using {computeProfile(*)}.
// Results are stored in {this->appr_vec}, {this->NodeInOrder, MtfCondProfile}, and {this->SizeGlobalMin, SizeFirstLocalMin}.
void MAPPR::computeAPPR(ProcessedGraph& graph_p, const int SeedNodeId, float alpha, float eps) {
  appr_vec.Clr();
  NodeWeightVH residual;
  NumPushs = 0;
  appr_norm = 0;
  if (graph_p.getNodeWeights(SeedNodeId).GetDat(SeedNodeId) * eps >= 1) {
    appr_vec(SeedNodeId) = 0;
    return;
  }
  residual(SeedNodeId) = 1;
  TSnapQueue<int> NodesWExcesRes;
  NodesWExcesRes.Push(SeedNodeId);

  while (!NodesWExcesRes.Empty()) {
    NumPushs += 1;
    int NodeId = NodesWExcesRes.Top();
    NodesWExcesRes.Pop();

    float deg_w = graph_p.getNodeWeights(NodeId).GetDat(NodeId);
    if (deg_w == 0) {
      appr_vec(NodeId) += residual(NodeId);
      appr_norm += residual(NodeId);
      residual(NodeId) = 0;
      continue;
    }
    float pushVal = residual(NodeId) - deg_w * eps / 2;
    appr_vec(NodeId) += pushVal * (1-alpha);
    appr_norm += pushVal * (1-alpha);
    residual(NodeId) = deg_w * eps / 2;
 
    pushVal *= alpha / deg_w;
    TUNGraph::TNodeI NI = graph_p.getTransformedGraph()->GetNI(NodeId);
    for (int i = 0; i < NI.GetOutDeg(); i ++) {
      int NbrId = NI.GetOutNId(i);
      float nbrValOld = residual(NbrId);
      float nbrValNew = nbrValOld + pushVal * graph_p.getNodeWeights(NodeId).GetDat(NbrId);
      residual(NbrId) = nbrValNew;
      if (nbrValOld <= eps * graph_p.getNodeWeights(NbrId).GetDat(NbrId)  &&  nbrValNew > eps * graph_p.getNodeWeights(NbrId).GetDat(NbrId)) {
        NodesWExcesRes.Push(NbrId);
      }
    }
  }
  computeProfile(graph_p);
}

// To compute the NCP of the graph with precomputed APPR vector
// Results are stored in {this->NodeInOrder and MtfCondProfile}.
// It will also compute the global min and first local min of the NCP.
void MAPPR::computeProfile(ProcessedGraph& graph_p) {
  NodeWeightVH Quotient;
  for (NodeWeightVH::TIter it = appr_vec.BegI(); it < appr_vec.EndI(); it++) {
    int NodeId = it->Key;
    Quotient(NodeId) = it->Dat / graph_p.getNodeWeights(NodeId).GetDat(NodeId);
  }
  Quotient.SortByDat(false);

  double vol = 0, cut = 0;
  TIntSet IsIn;           // the current set
  int VolSmall = 1;       // =1 if volume(IsIn) <= VolAll/2, and = -1 otherwise;

  for (NodeWeightVH::TIter it = Quotient.BegI(); it < Quotient.EndI(); it++) {
    int NodeId = it->Key;
    TUNGraph::TNodeI NI = graph_p.getTransformedGraph()->GetNI(NodeId);
    NodeWeightVH& WeightsHere = graph_p.getNodeWeights(NodeId);

    NodeInOrder.Add(NodeId);
    IsIn.AddKey(NodeId);
    vol += VolSmall * WeightsHere.GetDat(NodeId);
    if (VolSmall == 1 && graph_p.totalVolume_lte(vol*2)) {
      vol = graph_p.getTotalVolume() - vol;
      VolSmall = -1;
    }
    WeightsHere = graph_p.getNodeWeights(NodeId);
    cut += WeightsHere.GetDat(NodeId);
    for (long j = 0; j < NI.GetOutDeg(); j ++) {
      long NbrId = NI.GetOutNId(j);
      if (IsIn.IsKey(NbrId)) {
        cut -= 2 * WeightsHere.GetDat(NbrId);
      }
    }
    if (vol) {
      MtfCondProfile.Add(cut / vol);
    } else {
      MtfCondProfile.Add(1);
    }
  }
  findGlobalMin();
  findFirstlocalMin();
}

// Functions to find the global min and first local min of NCP.
bool MAPPR::isLocalMin(int idx, double thresh) {
    if (idx <= 0 || idx >= MtfCondProfile.Len() - 1) {
        return false;
    }
    if (MtfCondProfile[idx] >= MtfCondProfile[idx-1]) {
        return false;
    }
    int idxRight = idx;
    while (idxRight < MtfCondProfile.Len() - 1) {
        idxRight ++;
        if (MtfCondProfile[idxRight] > MtfCondProfile[idx] * thresh) {
            return true;
        } else if (MtfCondProfile[idxRight] <= MtfCondProfile[idx]) {
            return false;
        }
    }
    return false;
}
void MAPPR::findGlobalMin() {
  double minCondVal = 2;
  for (int i = 0; i < MtfCondProfile.Len(); i ++) {
    if (MtfCondProfile[i] < minCondVal) {
      SizeGlobalMin = i + 1;
      minCondVal = MtfCondProfile[i];
    }
  } 
}
void MAPPR::findFirstlocalMin() {
  SizeFirstLocalMin = 2;
  while (SizeFirstLocalMin < MtfCondProfile.Len()) {
    if (isLocalMin(SizeFirstLocalMin-1)) {
      break;
    }
    SizeFirstLocalMin ++;
  }
  if (SizeFirstLocalMin >= MtfCondProfile.Len()) {   // If there is no local min, we take the global min
    if (SizeGlobalMin == 0) {
      findGlobalMin();
    }
    SizeFirstLocalMin = SizeGlobalMin;
  }
}

// Function to find the desired cluster based on the NCP.
// Option > 0 will give the cluster of size option;
// Option = 0 will give the cluster of global minimum conductance;
// Option = -1 will give the cluster of first local minimum in the NCP.
// Result is stored in {TIntV Cluster}.
// Note that this function can only be run after finishing {computeAPPR(...)}
void MAPPR::sweepAPPR(int option) {
  if (appr_vec.Len() == 0) {
    TExcept::Throw("No APPR vector has been computed! Please first do MAPPR::computeAPPR(...)!");
  }

  if (option == 0) {  // use global min as output
    if (SizeGlobalMin == 0) {
      TExcept::Throw("A bug somewhere!");
    }
    sweepAPPR(SizeGlobalMin);
  } else if (option == -1) {
    if (SizeFirstLocalMin == -1) {
      TExcept::Throw("A bug somewhere!");
    }
    sweepAPPR(SizeFirstLocalMin);
  } else if (option > 0) {
    Cluster.Clr();
    for (int i = 0; i < option; i++) {
      Cluster.Add(NodeInOrder[i]);
    }
  } else {
    TExcept::Throw("Invalid input in option!");
  }
}



void MAPPR::printAPPR() {
  for (NodeWeightVH::TIter it = appr_vec.BegI(); it < appr_vec.EndI(); it++) {
    int NodeId = it->Key;
    float VecVal = it->Dat;
    printf("%d : %.7f\n", NodeId, VecVal);
  }
  printf("Number of pushes: %d\n", NumPushs);
  printf("L1-norm of APPR vector: %.7f\n", appr_norm);
}

void MAPPR::printProfile() {
  if (NodeInOrder.Len() == 0) {
    TExcept::Throw("No APPR vector has been computed! Please first do MAPPR::computeAPPR(...)!");
  }
  printf("Size\tNodeId\tConductance\n");
  for (int i = 0; i < NodeInOrder.Len(); i ++) {
    int NodeId = NodeInOrder[i];
    float Cond = MtfCondProfile[i];
    printf("%d\t%d\t%.7f\n", i+1, NodeId, Cond);
    if (i == SizeGlobalMin - 1) {
      printf("\tGlobal minimun!!!\n");
    }
    if (i == SizeFirstLocalMin - 1) {
      printf("\tFirst local minimun!!!\n");
    }
  }
}