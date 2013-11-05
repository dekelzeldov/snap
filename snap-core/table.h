#ifndef TABLE_H
#define TABLE_H
#include "predicate.h"
#include "tmetric.h"

class TTable;
typedef TPt<TTable> PTable;
class TTableContext;

typedef TPair<TIntV, TFltV> TGroupKey;

/*
This class serves as a wrapper for all data that needs to be shared by
several tables in an execution context
*/
class TTableContext{
protected:
  TStrHash<TInt, TBigStrPool> StringVals;
  friend class TTable;
public:
  TTableContext() {}
  TTableContext(TSIn& SIn): StringVals(SIn) {}
  void Save(TSOut& SOut) { StringVals.Save(SOut);}
};

/* 
TTable is a class representing an in-memory relational table with columnar data storage
*/
class TTable{
/******** Various typedefs / constants ***********/
public:
  /* possible policies for aggregating node attributes */
  typedef enum {MIN, MAX, FIRST, LAST, AVG, MEAN} ATTR_AGGR;
  /* possible operations on columns */
  typedef enum {OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD} OPS;

  /* a table schema is a vector of pairs <attribute name, attribute type> */
  typedef TVec<TPair<TStr, TAttrType> > Schema; 
protected:
  // special values for Next column
  static const TInt Last;
  static const TInt Invalid;

/************* Iterator classes ************/
public:
  // An iterator class to iterate over all currently existing rows
  // Iteration over the rows should be done using only this iterator
  class TRowIterator{
    TInt CurrRowIdx;
    const TTable* Table; 
  public:
    TRowIterator(): CurrRowIdx(0), Table(NULL){}
    TRowIterator(TInt RowIdx, const TTable* TablePtr): CurrRowIdx(RowIdx), Table(TablePtr){}
    TRowIterator(const TRowIterator& RowI): CurrRowIdx(RowI.CurrRowIdx), Table(RowI.Table){}
    TRowIterator& operator++(int){
      CurrRowIdx = Table->Next[CurrRowIdx];
      Assert(CurrRowIdx != Invalid);
      return *this;
    }
    bool operator < (const TRowIterator& RowI) const{ 
      if(CurrRowIdx == TTable::Last){ return false;}
      if(RowI.CurrRowIdx == TTable::Last){ return true;}
      return CurrRowIdx < RowI.CurrRowIdx;
    }
    bool operator == (const TRowIterator& RowI) const{ return CurrRowIdx == RowI.CurrRowIdx;}
    TInt GetRowIdx() const { return CurrRowIdx;}
    // we do not check column type in the iterator
    TInt GetIntAttr(TInt ColIdx) const{ return Table->IntCols[ColIdx][CurrRowIdx];}
    TFlt GetFltAttr(TInt ColIdx) const{ return Table->FltCols[ColIdx][CurrRowIdx];}
    TStr GetStrAttr(TInt ColIdx) const{ return Table->GetStrVal(ColIdx, CurrRowIdx);}
    TInt GetStrMap(TInt ColIdx) const{ return Table->StrColMaps[ColIdx][CurrRowIdx];}
    TInt GetIntAttr(const TStr& Col) const{ TInt ColIdx = Table->ColTypeMap.GetDat(Col).Val2; return Table->IntCols[ColIdx][CurrRowIdx];}
    TFlt GetFltAttr(const TStr& Col) const{ TInt ColIdx = Table->ColTypeMap.GetDat(Col).Val2; return Table->FltCols[ColIdx][CurrRowIdx];}
    TStr GetStrAttr(const TStr& Col) const{ return Table->GetStrVal(Col, CurrRowIdx);}   
    TInt GetStrMap(const TStr& Col) const{ TInt ColIdx = Table->ColTypeMap.GetDat(Col).Val2; return Table->StrColMaps[ColIdx][CurrRowIdx];}
  };

  /* an iterator that also allows logical row removal while iterating */
  class TRowIteratorWithRemove{
    TInt CurrRowIdx;
    TTable* Table;
    TBool Start;
  public:
    public:
    TRowIteratorWithRemove(): CurrRowIdx(0), Table(NULL), Start(true){}
    TRowIteratorWithRemove(TInt RowIdx, TTable* TablePtr): CurrRowIdx(RowIdx), Table(TablePtr), Start(RowIdx == TablePtr->FirstValidRow){}
    TRowIteratorWithRemove(TInt RowIdx, TTable* TablePtr, TBool IsStart): CurrRowIdx(RowIdx), Table(TablePtr), Start(IsStart){}
    TRowIteratorWithRemove(const TRowIteratorWithRemove& RowI): CurrRowIdx(RowI.CurrRowIdx), Table(RowI.Table), Start(RowI.Start){}
    TRowIteratorWithRemove& operator++(int){
      CurrRowIdx = GetNextRowIdx();
      Start = false;
      Assert(CurrRowIdx != Invalid);
      return *this;
    }
    bool operator < (const TRowIteratorWithRemove& RowI) const{ 
      if(CurrRowIdx == TTable::Last){ return false;}
      if(RowI.CurrRowIdx == TTable::Last){ return true;}
      return CurrRowIdx < RowI.CurrRowIdx;
    }
    bool operator == (const TRowIteratorWithRemove& RowI) const{ return CurrRowIdx == RowI.CurrRowIdx;}
    TInt GetRowIdx() const{ return CurrRowIdx;}
    TInt GetNextRowIdx() const { return (Start ? Table->FirstValidRow : Table->Next[CurrRowIdx]);}
    // we do not check column type in the iterator
    TInt GetNextIntAttr(TInt ColIdx) const{ return Table->IntCols[ColIdx][GetNextRowIdx()];}
    TFlt GetNextFltAttr(TInt ColIdx) const{ return Table->FltCols[ColIdx][GetNextRowIdx()];}
    TStr GetNextStrAttr(TInt ColIdx) const{ return Table->GetStrVal(ColIdx, GetNextRowIdx());}   
    TInt GetNextIntAttr(const TStr& Col) const{ TInt ColIdx = Table->ColTypeMap.GetDat(Col).Val2; return Table->IntCols[ColIdx][GetNextRowIdx()];}
    TFlt GetNextFltAttr(const TStr& Col) const{ TInt ColIdx = Table->ColTypeMap.GetDat(Col).Val2; return Table->FltCols[ColIdx][GetNextRowIdx()];}
    TStr GetNextStrAttr(const TStr& Col) const{ return Table->GetStrVal(Col, GetNextRowIdx());}   
    TBool IsFirst() const { return CurrRowIdx == Table->FirstValidRow;}
    void RemoveNext();
  };

/*************** TTable object fields ********************/
public:
  // Table name
  TStr Name;
protected:
  // Execution Context
  TTableContext& Context;
  // string pool for string values (part of execution context)
  // TStrHash<TInt, TBigStrPool>& StrColVals; 
  // use Context.StringVals to access the global string pool

  // Table Schema
  Schema S;
  // Reference Counter for Garbage Collection
  TCRef CRef;

  // number of rows in the table (valid and invalid)
  TInt NumRows;
  // number of valid rows in the table (i.e. rows that were not logically removed)
  TInt NumValidRows;
  // physical index of first valid row
  TInt FirstValidRow;
  // A vactor describing the logical order of the rows: Next[i] is the successor of row i
  // Table iterators follow the order dictated by Next
  TIntV Next; 

  // The actual columns - divided by types
	TVec<TIntV> IntCols;
	TVec<TFltV> FltCols;

  // string columns are implemented using a string pool to fight memory fragmentation
  // The value of string column c in row r is Context.StringVals.GetKey(StrColMaps[c][r])
	TVec<TIntV> StrColMaps; 
 
  // column name --> <column type, column index among columns of the same type>
	THash<TStr,TPair<TAttrType,TInt> > ColTypeMap;

  // name of column associated with permanent row id
  TStr IdColName;

  // hash table mapping permanent row ids to physical ids
  THash<TInt, TInt> RowIdMap;

  // group mapping data structures
  THash<TStr, TPair<TStrV, TBool> > GroupStmtNames;
  THash<TPair<TStrV, TBool>, THash<TInt, TGroupKey> >GroupIDMapping;
  THash<TPair<TStrV, TBool>, THash<TGroupKey, TIntV> >GroupMapping;

  // Fields to be used when constructing a graph
  // column to serve as src nodes when constructing the graph
  TStr SrcCol;
  // column to serve as dst nodes when constructing the graph
  TStr DstCol;
  // list of columns to serve as edge attributes
  TStrV EdgeAttrV;
  // list of columns to serve as source node attributes
  TStrV SrcNodeAttrV;
  // list of columns to serve as source node attributes
  TStrV DstNodeAttrV;
  // list of attributes pairs with values common to source and destination
  // and their common name. for instance: <T_1.age,T_2.age, age>
  //- T_1.age is a src node attribute, T_2.age is a dst node attribute.
  // However, since all nodes refer to the same universe of entities (users)
  // we just do one assignment of age per node
  // This list should be very small...
  TVec<TStrTr> CommonNodeAttrs;
  // Node values - i.e. the unique values of src/dst col
  TIntV IntNodeVals;
  THash<TFlt, TInt> FltNodeVals;
  TIntV StrNodeVals; // StrColMaps mappings

/***** value getters - getValue(column name, physical row Idx) *****/
public:
  // no type checking. assuming ColName actually refers to the right type.
  TInt GetIntVal(const TStr& ColName, TInt RowIdx){ return IntCols[ColTypeMap.GetDat(ColName).Val2][RowIdx];}
  TFlt GetFltVal(const TStr& ColName, TInt RowIdx){ return FltCols[ColTypeMap.GetDat(ColName).Val2][RowIdx];}
  TStr GetStrVal(const TStr& ColName, TInt RowIdx) const{ return GetStrVal(ColTypeMap.GetDat(ColName).Val2, RowIdx);}

/***** Utility functions *****/
protected:
  /* Utility functions for columns */
  void AddIntCol(const TStr& ColName);
  void AddFltCol(const TStr& ColName);

/***** Utility functions for handling string values *****/
  TStr GetStrVal(TInt ColIdx, TInt RowIdx) const{ return TStr(Context.StringVals.GetKey(StrColMaps[ColIdx][RowIdx]));}
  void AddStrVal(const TInt ColIdx, const TStr& Val);
  void AddStrVal(const TStr& Col, const TStr& Val);

/***** Utility functions for handling Schema *****/
  TStr GetSchemaColName(TInt Idx) const{ return S[Idx].Val1;}
  TAttrType GetSchemaColType(TInt Idx) const{ return S[Idx].Val2;}
  void AddSchemaCol(const TStr& ColName, TAttrType ColType) { S.Add(TPair<TStr,TAttrType>(ColName, ColType));}
  TInt GetColIdx(const TStr& ColName) const{ return ColTypeMap.IsKey(ColName) ? ColTypeMap.GetDat(ColName).Val2 : TInt(-1);}  // column index among columns of the same type
  TBool IsAttr(const TStr& Attr);

/***** Utility functions for adding attributes to the graph *****/
  // Get node identifier for src/dst node given row physical id
  TInt GetNId(const TStr& Col, TInt RowIdx);
  // add names of columns to be used as graph attributes
  void AddGraphAttribute(const TStr& Attr, TBool IsEdge, TBool IsSrc, TBool IsDst);
  void AddGraphAttributeV(TStrV& Attrs, TBool IsEdge, TBool IsSrc, TBool IsDst);

 // Add the attribute values to the graph - called by ToGraph
 // TODO: pass an aggregation policy per attribute - i.e. parameter of type THash<TStr,ATTR_AGGR>
  void AddNodeAttributes(PNEANet& Graph, ATTR_AGGR = LAST);
  void AddEdgeAttributes(PNEANet& Graph);
  // helper functions used by AddNodeAttributes
  /* Takes as parameters, and updates, maps NodeXAttrs: attribute name --> (Node Id --> attribute value) */
  void AddNodeAttributesAux(THash<TStr, TIntIntVH>& NodeIntAttrs, THash<TStr, TIntFltVH>& NodeFltAttrs, THash<TStr, TIntStrVH>& NodeStrAttrs, TBool Src);
  /* aggrgate vactor into a single scalar value according to a policy;
     used for choosing an attribute value for a node when this node appears in
     several records and has conflicting attribute values */     
  template <class T> 
  T AggregateVector(TVec<T>& V, ATTR_AGGR Policy){
    switch(Policy){
      case MIN:{
        T Res = V[0];
        for(TInt i = 1; i < V.Len(); i++){
          if(V[i] < Res){ Res = V[i];}
        }
        return Res;
      }
      case MAX:{
        T Res = V[0];
        for(TInt i = 1; i < V.Len(); i++){
          if(V[i] > Res){ Res = V[i];}
        }
        return Res;
      }
      case FIRST:{
        return V[0];
      }
      case LAST:{
        return V[V.Len()-1];
      }
      case AVG:{
        T Res = V[0];
        for(TInt i = 1; i < V.Len(); i++){
          Res = Res + V[i];
        }
        Res = Res / V.Len();
        return Res;
      }
      case MEAN:{
        V.Sort();
        return V[V.Len()/2];
      }
    }
    // added to remove a compiler warning
    T ShouldNotComeHere;
    return ShouldNotComeHere;
  }

  // preparation for graph generation of final table: retrieve the values of nodes (XNodeVals) - called by ToGraph
  void GraphPrep();
  // build graph out of the final table, without any attribute values - called by ToGraph
  void BuildGraphTopology(PNEANet& Graph);

  /***** Grouping Utility functions *************/
	// Group/hash by a single column with integer values. Returns hash table with grouping.
  // IndexSet tells what rows to consider (vector of physical row ids). It is used only if All == true.
  // Note that the IndexSet option is currently not used anywhere...
	void GroupByIntCol(const TStr& GroupBy, THash<TInt,TIntV>& grouping, const TIntV& IndexSet, TBool All) const; 
	// Group/hash by a single column with float values. Returns hash table with grouping.
	void GroupByFltCol(const TStr& GroupBy, THash<TFlt,TIntV>& grouping, const TIntV& IndexSet, TBool All) const;
	// Group/hash by a single column with string values. Returns hash table with grouping.
	void GroupByStrCol(const TStr& GroupBy, THash<TStr,TIntV>& grouping, const TIntV& IndexSet, TBool All) const;

 /* template for utility function to update a grouping hash map */
 template <class T>
  void UpdateGrouping(THash<T,TIntV>& Grouping, T Key, TInt Val) const{
    if(Grouping.IsKey(Key)){
      Grouping.GetDat(Key).Add(Val);
    } else{
      TIntV NewGroup;
      NewGroup.Add(Val);
      Grouping.AddDat(Key, NewGroup);
    }
  }

  /***** Utility functions for sorting by columns *****/
  // returns positive value if R1 is bigger, negative value if R2 is bigger, and 0 if they are equal (strcmp semantics)
  TInt CompareRows(TInt R1, TInt R2, const TStr& CompareBy);
  TInt CompareRows(TInt R1, TInt R2, const TStrV& CompareBy); 
  TInt GetPivot(TIntV& V, TInt StartIdx, TInt EndIdx, const TStrV& SortBy);
  TInt Partition(TIntV& V, TInt StartIdx, TInt EndIdx, const TStrV& SortBy);
  void ISort(TIntV& V, TInt StartIdx, TInt EndIdx, const TStrV& SortBy);
  void QSort(TIntV& V, TInt StartIdx, TInt EndIdx, const TStrV& SortBy);

  bool IsRowValid(TInt RowIdx) const{ return Next[RowIdx] != Invalid;}
  TInt GetLastValidRowIdx();

/***** Utility functions for removing rows (not through iterator) *****/
  void RemoveFirstRow();
  void RemoveRow(TInt RowIdx);
  void RemoveRows(const TIntV& RemoveV);
  // remove all rows that are not mentioned in the SORTED vector KeepV
  void KeepSortedRows(const TIntV& KeepV);

/***** Utility functions for Join *****/
  // Initialize an empty table for the join of this table with the given table
  PTable InitializeJointTable(const TTable& Table);
  // add joint row T1[RowIdx1]<=>T2[RowIdx2]
  void AddJointRow(const TTable& T1, const TTable& T2, TInt RowIdx1, TInt RowIdx2);

public:
/***** Constructors *****/
	TTable(); 
  TTable(TTableContext& Context);
  TTable(const TStr& TableName, const Schema& S, TTableContext& Context);
  TTable(TSIn& SIn, TTableContext& Context);
  TTable(const TTable& Table): Name(Table.Name), Context(Table.Context), S(Table.S),
    NumRows(Table.NumRows), NumValidRows(Table.NumValidRows), FirstValidRow(Table.FirstValidRow),
    Next(Table.Next), IntCols(Table.IntCols), FltCols(Table.FltCols),
    StrColMaps(Table.StrColMaps), ColTypeMap(Table.ColTypeMap), 
    GroupMapping(Table.GroupMapping),
    SrcCol(Table.SrcCol), DstCol(Table.DstCol),
    EdgeAttrV(Table.EdgeAttrV), SrcNodeAttrV(Table.SrcNodeAttrV),
    DstNodeAttrV(Table.DstNodeAttrV), CommonNodeAttrs(Table.CommonNodeAttrs){} 

  static PTable New(){ return new TTable();}
  static PTable New(TTableContext& Context){ return new TTable(Context);}
  static PTable New(const TStr& TableName, const Schema& S, TTableContext& Context){ return new TTable(TableName, S, Context);}
  static PTable New(const PTable Table){ return new TTable(*Table);}
  static PTable New(const PTable Table, const TStr& TableName){ PTable T = New(Table); T->Name = TableName; return T;}

/***** Save / Load functions *****/
  // Load table from spread sheet (TSV, CSV, etc)
  static PTable LoadSS(const TStr& TableName, const Schema& S, const TStr& InFNm, TTableContext& Context, const char& Separator = '\t', TBool HasTitleLine = true);
  // Load table from spread sheet - but only load the columns specified by RelevantCols
  static PTable LoadSS(const TStr& TableName, const Schema& S, const TStr& InFNm, TTableContext& Context, const TIntV& RelevantCols, const char& Separator = '\t', TBool HasTitleLine = true);
  // Save table schema + content into a TSV file
  void SaveSS(const TStr& OutFNm);
  // Load table from binary. The TTableContext must be provided separately as it shared among multiple TTables and should be saved in a separate binary.
  static PTable Load(TSIn& SIn, TTableContext& Context){ return new TTable(SIn, Context);} 
  // Save table schema + content into binary. Note that TTableContext must be saved in a separate binary (as it is shared among multiple TTables).
	void Save(TSOut& SOut);

/***** Graph handling *****/
  /* Create a graph out of the FINAL table */
	PNEANet ToGraph(ATTR_AGGR AttrAggrPolicy = LAST);

  /* Getters and Setters of data required for building a graph out of the table */
	TStr GetSrcCol() const { return SrcCol; }
  void SetSrcCol(const TStr& Src) {
    if(!ColTypeMap.IsKey(Src)){TExcept::Throw(Src + ": no such column");}
    SrcCol = Src;
  }
	TStr GetDstCol() const { return DstCol; }
  void SetDstCol(const TStr& Dst) {
    if(!ColTypeMap.IsKey(Dst)){TExcept::Throw(Dst + ": no such column");}
    DstCol = Dst;
  }
  /* Specify table attributes (columns) as graph attributes */
  void AddEdgeAttr(const TStr& Attr){AddGraphAttribute(Attr, true, false, false);}
  void AddEdgeAttr(TStrV& Attrs){AddGraphAttributeV(Attrs, true, false, false);}
  void AddSrcNodeAttr(const TStr& Attr){AddGraphAttribute(Attr, false, true, false);}
  void AddSrcNodeAttr(TStrV& Attrs){AddGraphAttributeV(Attrs, false, true, false);}
  void AddDstNodeAttr(const TStr& Attr){AddGraphAttribute(Attr, false, false, true);}
  void AddDstNodeAttr(TStrV& Attrs){AddGraphAttributeV(Attrs, false, false, true);}
  // handling the common case where source and destination both belong to the same "universe" of entities
  void AddNodeAttr(const TStr& Attr){AddSrcNodeAttr(Attr); AddDstNodeAttr(Attr);}
  void AddNodeAttr(TStrV& Attrs){AddSrcNodeAttr(Attrs); AddDstNodeAttr(Attrs);}
  void SetCommonNodeAttrs(const TStr& SrcAttr, const TStr& DstAttr, const TStr& CommonAttrName){ 
    CommonNodeAttrs.Add(TStrTr(SrcAttr, DstAttr, CommonAttrName));
  }
  /* getters for attribute name vectors */
	TStrV GetSrcNodeIntAttrV() const;
  TStrV GetDstNodeIntAttrV() const;
	TStrV GetEdgeIntAttrV() const;
	TStrV GetSrcNodeFltAttrV() const;
  TStrV GetDstNodeFltAttrV() const;
	TStrV GetEdgeFltAttrV() const;
	TStrV GetSrcNodeStrAttrV() const;
  TStrV GetDstNodeStrAttrV() const;
	TStrV GetEdgeStrAttrV() const;

/***** Basic Getters *****/
	TAttrType GetColType(const TStr& ColName) const{ return ColTypeMap.GetDat(ColName).Val1; };
  TInt GetNumRows() const { return NumRows;}
  TInt GetNumValidRows() const { return NumValidRows;}

  THash<TInt, TInt> GetRowIdMap() const { return RowIdMap;}
	
/***** Iterators *****/
  TRowIterator BegRI() const { return TRowIterator(FirstValidRow, this);}
  TRowIterator EndRI() const { return TRowIterator(TTable::Last, this);}
  TRowIteratorWithRemove BegRIWR(){ return TRowIteratorWithRemove(FirstValidRow, this);}
  TRowIteratorWithRemove EndRIWR(){ return TRowIteratorWithRemove(TTable::Last, this);}

/***** Table Operations *****/
	// rename / add a label to a column
	void AddLabel(const TStr& column, const TStr& newLabel);

	// Remove rows with duplicate values in given columns
  void Unique(const TStr& Col);
	void Unique(const TStrV& Cols, TBool Ordered = true);

	/* 
  Select. Has two modes of operation:
  1. If Filter == true then (logically) remove the rows for which the predicate doesn't hold
  2. If filter == false then add the physical indices of the rows for which the predicate holds to the vactor SelectedRows
  */
	void Select(TPredicate& Predicate, TIntV& SelectedRows, TBool Filter = true);
  void Select(TPredicate& Predicate){
    TIntV SelectedRows;
    Select(Predicate, SelectedRows);
  }
  // select atomic - optimized cases of select with predicate of an atomic form: compare attribute to attribute or compare attribute to a constant
  void SelectAtomic(const TStr& Col1, const TStr& Col2, TPredicate::COMP Cmp, TIntV& SelectedRows, TBool Filter = true);
  void SelectAtomic(const TStr& Col1, const TStr& Col2, TPredicate::COMP Cmp){
    TIntV SelectedRows;
    SelectAtomic(Col1, Col2, Cmp, SelectedRows);
  }
  void SelectAtomicIntConst(const TStr& Col1, TInt Val2, TPredicate::COMP Cmp, TIntV& SelectedRows, TBool Filter = true);
  void SelectAtomicIntConst(const TStr& Col1, TInt Val2, TPredicate::COMP Cmp){
    TIntV SelectedRows;
    SelectAtomicIntConst(Col1, Val2, Cmp, SelectedRows);
  }
  void SelectAtomicStrConst(const TStr& Col1, const TStr& Val2, TPredicate::COMP Cmp, TIntV& SelectedRows, TBool Filter = true);
  void SelectAtomicStrConst(const TStr& Col1, const TStr& Val2, TPredicate::COMP Cmp){
    TIntV SelectedRows;
    SelectAtomicStrConst(Col1, Val2, Cmp, SelectedRows);
  }
  void SelectAtomicFltConst(const TStr& Col1, TFlt Val2, TPredicate::COMP Cmp, TIntV& SelectedRows, TBool Filter = true);
  void SelectAtomicFltConst(const TStr& Col1, TFlt Val2, TPredicate::COMP Cmp){
    TIntV SelectedRows;
    SelectAtomicFltConst(Col1, Val2, Cmp, SelectedRows);
  }

  // store column for a group, physical row ids have to be passed
  void StoreGroupCol(const TStr& GroupColName, const TVec<TPair<TInt, TInt> >& GroupAndRowIds);

  // if keep unique is true, unique vec will be modified to contain a row from each group
  // if keep unique is false, then normal grouping is done and a new column is added depending on 
  // whether groupcolname is an empty string
  void GroupAux(const TStrV& GroupBy, THash<TGroupKey, TPair<TInt, TIntV> >& Grouping, TBool Ordered,
    const TStr& GroupColName, TBool KeepUnique, TIntV& UniqueVec);

  // group - specify columns to group by, name of column in new table, whether to treat columns as ordered
  // if name of column is an empty string, no column is created
	void Group(const TStrV& GroupBy, const TStr& GroupColName, TBool Ordered = true);
	
	// count - count the number of appearences of the different elements of col
	// record results in column CountCol
	void Count(const TStr& CountColName, const TStr& Col);
	// order the rows according to the values in columns of OrderBy (in descending
	// lexicographic order). 
	void Order(const TStrV& OrderBy, const TStr& OrderColName = "", TBool ResetRankByMSC = false);

	// perform equi-join with given columns - i.e. keep tuple pairs where 
	// this->Col1 == Table->Col2; Implementation: Hash-Join - build a hash out of the smaller table
	// hash the larger table and check for collisions
	PTable Join(const TStr& Col1, const TTable& Table, const TStr& Col2);
  PTable SelfJoin(const TStr& Col){return Join(Col, *this, Col);}

	// compute distances between elements in this->Col1 and Table->Col2 according
	// to given metric. Store the distances in DistCol, but keep only rows where
	// distance <= threshold
	//void Dist(const TStr& Col1, const TTable& Table, const TStr Col2, const TStr& DistColName, const TMetric& Metric, TFlt threshold);

  // Release memory of deleted rows, and defrag
  // also updates meta-data as row indices have changed
  // need some liveness analysis of columns
  void Defrag();
  
  // Add all the rows of the input table (which ,ust have the same schema as current table) - allows duplicate rows (not a union)
  void AddTable(const TTable& T);
  
  void AddRow(const TRowIterator& RI);
  void GetCollidingRows(const TTable& T, THashSet<TInt>& Collisions) const;
  PTable Union(const TTable& Table, const TStr& TableName);
  PTable Intersection(const TTable& Table, const TStr& TableName);
  PTable Minus(const TTable& Table, const TStr& TableName);
  PTable Project(const TStrV& ProjectCols, const TStr& TableName);
  void ProjectInPlace(const TStrV& ProjectCols);
  
  /* Column-wise arithmetic operations */

  /*
   * Performs Attr1 OP Attr2 and stores it in Attr1
   * If ResAttr != "", result is stored in a new column ResAttr
   */
  void ColGenericOp(const TStr& Attr1, const TStr& Attr2, const TStr& ResAttr, OPS op);
  void ColAdd(const TStr& Attr1, const TStr& Attr2, const TStr& ResultAttrName="");
  void ColSub(const TStr& Attr1, const TStr& Attr2, const TStr& ResultAttrName="");
  void ColMul(const TStr& Attr1, const TStr& Attr2, const TStr& ResultAttrName="");
  void ColDiv(const TStr& Attr1, const TStr& Attr2, const TStr& ResultAttrName="");
  void ColMod(const TStr& Attr1, const TStr& Attr2, const TStr& ResultAttrName="");

  /* Performs Attr1 OP Attr2 and stores it in Attr1 or Attr2
   * This is done depending on the flag AddToFirstTable
   * If ResAttr != "", result is stored in a new column ResAttr
   */
  void ColGenericOp(const TStr& Attr1, TTable& Table, const TStr& Attr2, const TStr& ResAttr, 
    OPS op, TBool AddToFirstTable);
  void ColAdd(const TStr& Attr1, TTable& Table, const TStr& Attr2, const TStr& ResAttr="",
    TBool AddToFirstTable=true);
  void ColSub(const TStr& Attr1, TTable& Table, const TStr& Attr2, const TStr& ResAttr="",
    TBool AddToFirstTable=true);
  void ColMul(const TStr& Attr1, TTable& Table, const TStr& Attr2, const TStr& ResAttr="",
    TBool AddToFirstTable=true);
  void ColDiv(const TStr& Attr1, TTable& Table, const TStr& Attr2, const TStr& ResAttr="",
    TBool AddToFirstTable=true);
  void ColMod(const TStr& Attr1, TTable& Table, const TStr& Attr2, const TStr& ResAttr="",
    TBool AddToFirstTable=true);

  /* Performs Attr1 OP Num and stores it in Attr1
   * If ResAttr != "", result is stored in a new column ResAttr
   */
  void ColGenericOp(const TStr& Attr1, TFlt Num, const TStr& ResAttr, OPS op);
  void ColAdd(const TStr& Attr1, TFlt Num, const TStr& ResultAttrName="");
  void ColSub(const TStr& Attr1, TFlt Num, const TStr& ResultAttrName="");
  void ColMul(const TStr& Attr1, TFlt Num, const TStr& ResultAttrName="");
  void ColDiv(const TStr& Attr1, TFlt Num, const TStr& ResultAttrName="");
  void ColMod(const TStr& Attr1, TFlt Num, const TStr& ResultAttrName="");

  // add explicit row ids, initialise hash set mapping ids to physical rows
  void InitIds();

  // add a column of explicit integer identifiers to the rows
  void AddIdColumn(const TStr& IdColName);

  // creates a table T' where the rows are joint rows (T[r1],T[r2]) such that 
  // r2 is one of the successive rows to r1 when this table is ordered by OrderCol,
  // and both r1 and r2 have the same value of GroupBy column
  PTable IsNextK(const TStr& OrderCol, TInt K, const TStr& GroupBy, const TStr& RankColName = "");

  // debug print sizes of various fields of table
  void PrintSize();

  friend class TPt<TTable>;
};

typedef TPair<TStr,TAttrType> TStrTypPr;
#endif //TABLE_H
