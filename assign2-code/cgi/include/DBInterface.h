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

  /**
  * @param datasourceID the index of the database used for this search
  * @param results pointer to the vector of results
  * @param listLength the max. number of results to show on this page
  * @param rankStart the starting number of the first result of the page
  */
  void displaySearchResults(lemur::api::Index *db, int datasourceID, lemur::parse::StringQuery* q,
                            lemur::api::IndexedRealVector *results, int listLength, int rankStart);

  /**
   * Creates a summary string for the result
   *
   */
  std::string getSummaryString(const lemur::api::DocumentManager* dm, lemur::api::Index *db, lemur::parse::StringQuery* q, int resultID, string docext);

  /**
   * Retrieves the original HTTP link of the document for trecweb data
   *
   * @param index the opened index
   * @param docID the document internal ID
   * @return the original URI
   */
  string getDocHTTPLink(lemur::api::Index *index, long docID);


public:
  enum QUERY_INTERFACE_TYPE {
    QUERY_INTERFACE_INDRI=1,
    QUERY_INTERFACE_INQUERY
  };
  /**
   * Basic constructor.
   */
  DBInterface();

  /**
   * Constructor w/ specific CGI output
   * @param outputInterface the output object
   */
  DBInterface(CGIOutput *outputInterface);

  /**
   * Destructor
   */
  ~DBInterface();

  /**
   * Sets the base root path
   * @param rootPath the root path
   */
  void setDataRoot(string rootPath);

  /**
   * Sets the path to the index
   * @param indexPath the path to the index
   */
  void setIndexPath(string indexPath);

  /**
   * Sets the output interface
   * @param outputInterface the output object
   */
  void setOutput(CGIOutput *outputInterface);

  /**
   * Displays the index listing
   */
  void displayIndexStatistics();

  /**
   * Returns the current data root path (if set)
   * @return the data root path
   */
  string getDataRoot();

  /**
   * Returns the current index path
   * @return the current index path
   */
  string getIndexPath();

  /**
   * Performs a search and outputs to the output object
   * @param datasourceID which index to use
   * @param query the actual raw query
   * @param listLength the maximum number of results per page
   * @param rankStart the starting number to use for our results
   * @param queryType the query language type
   */
  void search(int datasourceID, string *query, long listLength, long rankStart);

  /**
   * Retrieves a parsed document and outputs it
   * @param docID the internal Lemur ID of the document
   */
  void getParsedDoc(long docID);

  /**
   * Retrieves a raw document and outputs it
   * @param docID the internal Lemur ID of the document
   */
  void getDocIID(long docID);

  /**
   * Retrieves a document from the original document's ID and outputs it
   * @param docID the external document ID
   */
  void getDocXID(string *docID);

  /**
   * Outputs the corpus statistics for a given term
   * @param term the term string
   */
  void getTermCorpusStats(string *term);


  /**
   * Outputs the stemmed version (if any) of the given word
   * @param term the term string
   */
  void getWordStem(string *term);

  double computeWeight(int docID, int termID,  int docTermFreq, int qryTermProb, Index & ind);
  double computeAdjustedScore(double origScore, int docID, int qryProbSum, Index &ind);
  void retrieve(int * query, Index &ind, ScoreAccumulator &scAcc, IndexedRealVector &results);

}; // class DBInterface

#endif // _DBINTERFACE_H

