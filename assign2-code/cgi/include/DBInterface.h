#ifndef _DBINTERFACE_H
#define _DBINTERFACE_H

#include <string>
using std::string;

#include "CGIOutput.h"
#include "Index.hpp"
#include "IndexManager.hpp"
#include "Stemmer.hpp"
#include "IndexManager.hpp"
#include "Stopper.hpp"
#include "StringQuery.hpp"
#include "ElemDocMgr.hpp"
#include "MatchInfo.hpp"
#include "TextHandlerManager.hpp"
#include "Param.hpp"
#include "ScoreAccumulator.hpp"

using namespace lemur::api;

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

/**
 * A class that represents a single result item for the results
 *
 * @author Mark J. Hoy [http://www.cs.cmu.edu/~mhoy/]
 * @version 4/14/06
 */
class DBInterface {

private:
  string    pathToIndex;
  string    dataRoot;
  CGIOutput *output;

  lemur::api::Stemmer* getDbStemmer(const lemur::api::Index* ind);

  lemur::api::Index *openIndex();

  lemur::api::IndexedRealVector removeDuplicateResults(lemur::api::IndexedRealVector results);
    void displaySearchResults(lemur::api::Index *db, int datasourceID, lemur::parse::StringQuery *q, lemur::api::IndexedRealVector *results, int listLength, int rankStart);
    std::string getSummaryString(const lemur::api::DocumentManager *dm, lemur::api::Index *db, lemur::parse::StringQuery *q, int resultID, string docext);
    string getDocHTTPLink(lemur::api::Index *index, long  docID);
    void addTermProximity(vector<int> qryTermIDs, IndexedRealVector & results, lemur::api::Index *& db, int *qt);

    // scoring-specific functions
    double computeIDFWeight(int termID, Index & ind);
    double computeTFWeight(int docID, int tf, Index & ind);
    double computeQTFWeight(int qtf);
    double computeDLWeight(int docID, Index & ind);

public:
    enum QUERY_INTERFACE_TYPE{ QUERY_INTERFACE_INDRI = 1, QUERY_INTERFACE_INQUERY};
    DBInterface();
    DBInterface(CGIOutput *outputInterface);
    ~DBInterface();
    void setDataRoot(string rootPath);
    void setIndexPath(string indexPath);
    void setOutput(CGIOutput *outputInterface);
    void displayIndexStatistics();
    string getDataRoot();
    string getIndexPath();
    void search(int datasourceID, string *query, long  listLength, long  rankStart);
    void getParsedDoc(long  docID);
    void getDocIID(long  docID);
    void getDocXID(string *docID);
    void getTermCorpusStats(string *term);
  void getWordStem(string *term);

  double computeWeight(int docID, int termID,  int docTermFreq, int qryTermProb, Index & ind);
  double computeAdjustedScore(double origScore, int docID, int qryProbSum, Index &ind);
  void retrieve(int * query, Index &ind, ScoreAccumulator &scAcc, IndexedRealVector &results);

}; // class DBInterface

#endif // _DBINTERFACE_H

