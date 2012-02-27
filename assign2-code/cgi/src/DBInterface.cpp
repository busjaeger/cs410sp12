#include "DBInterface.h"
#include "BasicCollectionProps.hpp"
#include "TrecParser.hpp"
#include <math.h>

using namespace lemur::api;

/** construction / destruction **/
DBInterface::DBInterface() {
  output=NULL;
}

DBInterface::DBInterface(CGIOutput *outputInterface) {
  output=outputInterface;
}

DBInterface::~DBInterface() {
}


/** private functions **/
lemur::api::Stemmer* DBInterface::getDbStemmer(const lemur::api::Index* ind) {
  Stemmer* stemmer = NULL;

  // get our collection properties
  const lemur::parse::BasicCollectionProps* props = dynamic_cast<const lemur::parse::BasicCollectionProps*> (ind->collectionProps());
  if (props) {
    string stype = "";
    const lemur::parse::Property* p = NULL;

    // get the stemmer propery...
    p = props->getProperty(Stemmer::category);
    if (p) {
      stype = (char*)p->getValue();
    }

    // create the stemmer...
    stemmer = TextHandlerManager::createStemmer(stype);
  }
  return stemmer;
}

lemur::api::Index *DBInterface::openIndex() {
  Index* db;

  try {
    db = IndexManager::openIndex(pathToIndex);
  }
  catch (...) {
    db = NULL;
  }
  return db;
}

string DBInterface::getDocHTTPLink(lemur::api::Index *index, long docID) {
  string retString("[no url]");

  if (!index) return retString;

  const DocumentManager* dm = NULL;
  string idstr;
  stringstream docidStrStr;
  docidStrStr << docID;

  if (! (dm = index->docManager (docID))) {
    return retString;
  } else {

    idstr = index->document (docID);

    if (idstr == "[OOV]") {
      return retString;
    }
  }

  char *theDoc=dm->getDoc(idstr);

  string docString(theDoc);

  std::string::size_type hdrPos=docString.find("<DOCHDR>");
  if (hdrPos > 0) {
    // safely assume it won't be at the beginning...
    std::string::size_type hdrPosEnd=docString.find("</DOCHDR>", hdrPos);
    if (hdrPosEnd > hdrPos) {
      // ensure the end doc header is after the beginning...
      std::string::size_type httpStartPos=docString.find("http://", hdrPos);
      if ((httpStartPos > hdrPos) && (httpStartPos < hdrPosEnd)) {
        // ensure our link is between the DOCHDR tags...
        // get the string to the end of the line...
        char *tmpString=strdup(docString.substr(httpStartPos, (hdrPosEnd-httpStartPos)).c_str());
        if (tmpString) {
          char *fTok=strtok(tmpString, " \t\n\r");
          if (fTok) {
            retString=fTok;
          } // if (fTok)
          delete tmpString;
        } // if (tmpString)
      } // end if ((httpStartPos > hdrPos) && (httpStartPos < hdrPosEnd))
    } // end if (hdrPosEnd > hdrPos)
  } // end if (hdrPos > 0)

  return retString;
}




/** public functions **/

void DBInterface::setDataRoot(string rootPath) {
  dataRoot=rootPath;
}

void DBInterface::setIndexPath(string indexPath) {
  pathToIndex=indexPath;
}

void DBInterface::setOutput(CGIOutput *outputInterface) {
  output=outputInterface;
}

string DBInterface::getDataRoot() {
  return dataRoot;
}

string DBInterface::getIndexPath() {
  return pathToIndex;
}

void DBInterface::displayIndexStatistics() {
  Index *db=openIndex();
  if (!db) {
    output->writeErrorMessage("Cannot open index.", "Error opening index: " + pathToIndex);
    return;
  }

  // get doc count
  long docCount=db->docCount();
  // get average document length
  double avgDocLen=db->docLengthAvg();
  // get unique terms
  long numUniqueTerms=db->termCountUnique();
  // get total terms
  long numTotalTerms=db->termCount();


  stringstream statsString;
  statsString << "Corpus Size: " << docCount << " document";
  if (docCount!=1) {
    statsString << "s";
  }
  statsString << "\n";
  statsString << "Corpus Length (in words): " << numTotalTerms << "\n"
    << "Unique Terms: " << numUniqueTerms << "\n"
    << "Average Document Length: " << avgDocLen << " words\n";

  output->displayDataPage(statsString.str(), "Index Statistics");

  delete db;
}

std::string DBInterface::getSummaryString(const lemur::api::DocumentManager* dm, lemur::api::Index *db,
                                          lemur::parse::StringQuery* q, int resultID, string docext) {
    //
    // Build excerpt from MatchInfo matches ------------------------------------------
    //

    std::string retSummary="";

    string doc="";
    char *tmpDoc=dm->getDoc(docext);
    if (tmpDoc) {
      doc.append(tmpDoc);
    }

    MatchInfo* mi = MatchInfo::getMatches(*db, *q, resultID);

    int matchcount = mi->count();
    //int docmatches[matchcount*2];
    int *docmatches = new int[matchcount*2]; //studio .NET doesn't like

    // mi->startIteration();
    for (int m=0; m < matchcount; m++) {
      TMatch tm = mi->at(m);
      //TMatch tm = mi->nextMatch();
      docmatches[m*2] = tm.start;
      docmatches[m*2+1] = tm.end;
    } // end for (int m=0; m < matchcount; m++)

    if (matchcount > 0) {
      // Display first three matches with passage30 for each, try to break on whitespace
      int start, end;  // bounds of passage
      int first, last; // occurrences of whitespace
      int finalpassage = matchcount;

      if (finalpassage > 3) {
        finalpassage = 3;
      }

      bool badTextOffset=false;

      for (int m=0; (!badTextOffset) && (m < finalpassage); m++) {
        start = docmatches[m*2] - 30;
        end = docmatches[m*2+1] + 30;

        if (start < 0) {
          start = 0;
        }

        if (end >= doc.length()) {
          end = doc.length() - 1;
        }

        first = start;
        last = end;

        for (int w = (docmatches[m*2] + start) / 2; w > start; w--) {
          if (w >= doc.length()) {
            badTextOffset=true;
          } else {
            if (doc.at(w) == ' ') first = w;
          }
        }

        for (int w = (docmatches[m*2+1] + end) / 2; w < end; w++) {
          if (w >= doc.length()) {
            badTextOffset=true;
          } else {
            if (doc.at(w) == ' ') last = w;
          }
        }

        if (!badTextOffset) {
          docmatches[m*2] = first;
          docmatches[m*2+1] = last;
        }
      } // end for (int m=0; m < finalpassage; m++)

      retSummary.append("...");
      if ((!badTextOffset) && (finalpassage > 1)) {
        //
        // Combine overlapping passages
        //
        for (int m=1; m < finalpassage; m++) {
          if (docmatches[m*2-1] >= docmatches[m*2]) {
            docmatches[m*2] = docmatches[m*2-2];
          } else {
            // no overlap so display previous passage
            string passage = doc.substr(docmatches[m*2-2],docmatches[m*2-1]+1 - docmatches[m*2-2]);
            retSummary.append(passage + "...");
          }
        } // end for (int m=1; m < finalpassage; m++)
      } // end if (finalpassage > 1)
      // Display last passage (max third)
      if (!badTextOffset) {
        std::string passage = doc.substr(docmatches[finalpassage*2-2],docmatches[finalpassage*2-1]+1 - docmatches[finalpassage*2-2]);
        retSummary.append(passage + "...");
      }
    } // end if (matchcount > 0)

    int pos = 0;
    while ((pos = retSummary.find('<',pos)) != string::npos) {
      retSummary.replace(pos, 1, "&lt;", 4);
    }

    pos = 0;
    while ((pos = retSummary.find('>',pos)) != string::npos) {
      retSummary.replace(pos, 1, "&gt;", 4);
    }

    // clean up
    delete[](docmatches);

    // Done with matches from MatchInfo ------------------------------------------
    return retSummary;
}

lemur::api::IndexedRealVector DBInterface::removeDuplicateResults(lemur::api::IndexedRealVector results) {
  // simple duplicate detection and removal...
  // based on Don Metzler's work - in theory, if two results have
  // the _exact_ same score, they should be duplicates....

  lemur::api::IndexedRealVector retVector;

  lemur::api::IndexedRealVector::iterator vIter=results.begin();
  // some riduculous seed value...
  double currentScore=9999999999.999999;
  while (vIter!=results.end()) {
    if ((*vIter).val!=currentScore) {
      retVector.push_back((*vIter));
      currentScore=(*vIter).val;
    }
    vIter++;
  }

  return retVector;
}

/**
 * @param datasourceID the index of the database used for this search
 * @param results pointer to the vector of results
 * @param listLength the max. number of results to show on this page
 * @param rankStart the starting number of the first result of the page
 */
void DBInterface::displaySearchResults(lemur::api::Index *db, int datasourceID, lemur::parse::StringQuery* q,
                                       lemur::api::IndexedRealVector *results,
                                       int listLength, int rankStart) {

  // start the page...

  output->setMaxResultsPerPage(listLength);

  //
  //  If someone tries to go past the end of the list, don't.
  //
  if (rankStart >= results->size()) {
    rankStart = results->size() - 1;
  }

  int maxResultsToGet=results->size();
  if (DEFAULT_MAX_DOCUMENTS_TO_RETRIEVE!=0) {
    maxResultsToGet=MIN(results->size(), DEFAULT_MAX_DOCUMENTS_TO_RETRIEVE);
  }

  output->setResultStatistics(  datasourceID, rankStart,
                                MIN(rankStart+listLength, maxResultsToGet),
                                maxResultsToGet
                              );

  output->displayResultsPageBeginning();

  stringstream htmlListStart;
  htmlListStart << "<ol type=1 start=\"" << (rankStart + 1) << "\">\n";
  output->outputString(htmlListStart.str());


  for (int i=rankStart;(i<listLength+rankStart) && (i<maxResultsToGet);i++) {

    //
    // get DocMgr
    //
    const DocumentManager* dm = db->docManager((*results)[i].ind);

    //
    // fetch possible title
    //
    string docext = db->document((*results)[i].ind);

    //
    // Get the summary item (if any)
    //
    string buf = getSummaryString(dm, db, q, (*results)[i].ind, docext);


    // not using an Indri index - use the old fashioned methods...
    const lemur::parse::ElemDocMgr* elem = dynamic_cast<const lemur::parse::ElemDocMgr*>(dm);
    if (elem) {
      char* ptitle = elem->getElement(docext.c_str(),"TITLE");
      char* pnotes = elem->getElement(docext.c_str(),"DATE");
      string notes = "";
      
      if (pnotes) {
	notes = pnotes;
      }
      
      if (!ptitle) {
	ptitle = elem->getElement(docext.c_str(), "HEADLINE");
      }
      
      if (ptitle) {
	output->writeSearchResult(docext, docext, ptitle, buf, (*results)[i].val, datasourceID, (*results)[i].ind);
	delete[]ptitle;
      } else {
	output->writeSearchResult(docext, docext, docext, buf, (*results)[i].val, datasourceID, (*results)[i].ind);
      } // end if (ptitle)
    } else {
      output->writeSearchResult(docext, docext, docext, buf, (*results)[i].val, datasourceID, (*results)[i].ind);
    } // end [else] if (elem)
  } // for (int i=rankStart;(i<listLength+rankStart) && (i<results.size());i++)

  output->outputString("</ol>\n");
  output->displayResultsPageEnding();
}


double DBInterface::computeWeight(int docID,
		     int termID,
		     int docTermFreq,
		     int qryTermFreq, 
		     Index & ind)
{
	double N = ind.docCount();
	double df = ind.docCount(termID);
	double tf = docTermFreq;
	double qtf = qryTermFreq;
	double dl = ind.docLength(docID);
	double avdl = ind.docLengthAvg();
	double k1 = 1.2;
	double b = 0.75;
	double k3 = 1000;

	return log((N - df + 0.5) / (df + 0.5))
			* (((k1 + 1.0) * tf) / (tf + k1 * (1.0 - b + b * (dl / avdl))))
			* (((k3 + 1.0) * qtf) / (k3 + qtf));
}

// compute the adjusted score
double DBInterface::computeAdjustedScore(double origScore, // the score from the accumulator
			    int docID, // doc ID
			    int qrySum, // number of query terms 
			    Index &ind) // index
{
	return origScore;
}



void DBInterface::retrieve(int * query, Index &ind, // index
	       ScoreAccumulator &scAcc, // score accumulator
	       IndexedRealVector &results) // result docs
{
  int t;

  // for each query, first reset the accumulator
  scAcc.reset();

  // then,  go through each query term to accumulate scores
	int qryFreqSum =0;
  for (t=1; t<=ind.termCountUnique();t++) {
    if (query[t]>0) {
	qryFreqSum+=query[t];
      // fetch inverted entries
      DocInfoList *dList = ind.docInfoList(t);

      // iterate over all entries
      dList->startIteration();
      while (dList->hasMore()) {
	DocInfo *info = dList->nextEntry();
	// for each entry, compute the weight contribution
	double wt = computeWeight(info->docID(),  // doc ID
				  t, // term ID
				  info->termCount(), // freq of term t in this doc
				  query[t], // freq. of term t in the query
				  ind);
	// increase the score for this document
	scAcc.incScore(info->docID(), wt);
      }
      delete dList;
    }
  }

  // now copy scores into the result data structure, adjust them if necessary.
  results.clear();
  double s;
  int d;
  for (d=1; d<=ind.docCount(); d++) {
    if (scAcc.findScore(d,s)) {
    } else {
      s=0;
    }
  results.PushValue(d, computeAdjustedScore(s, // the score from the accumulator
						     d, // doc ID
						     qryFreqSum, // total freq sum of the query terms.
					     ind)); // index


  }

}

// Method needed for search

void DBInterface::search(int datasourceID, string *query, long listLength, long rankStart)
{
  // open the index
  lemur::api::Index *db=openIndex();
  if (!db) {
    output->writeErrorMessage("Cannot open index.", "Error opening index: " + pathToIndex);
    return;
  }

  Stopper* stopper = NULL;
  Stemmer* stemmer = getDbStemmer(db);

  const lemur::parse::BasicCollectionProps* props = dynamic_cast<const lemur::parse::BasicCollectionProps*> (db->collectionProps());

  // create the results vector
  IndexedRealVector results(db->docCount());

  lemur::retrieval::ArrayAccumulator accumulator(db->docCount());


  if (props) {
    const lemur::parse::Property* p = NULL;
    p = props->getProperty("stopwords");
    if (p) {
      stopper = TextHandlerManager::createStopper((char*)p->getValue());
    }
  } // end if (props)


  
    // process InQuery query
  
  int qlen = query->length();
  char* qChar = new char[qlen+5];
  sprintf(qChar, "#q1=%s\0", query->c_str());
  lemur::parse::TrecParser opparser;
  lemur::parse::StringQuery* q = new lemur::parse::StringQuery(*query);
  TextHandler* th = &opparser;



	// create a query representation
    int *qt = new int[db->termCountUnique()+1];
     for (int t=1; t<=db->termCountUnique(); t++) {
	      qt[t]=0;
    }
    int qryFreqSum =0;
	// iteration through each term in the query
    q->startTermIteration();
    while (q->hasMore()) {
	   const Term *qryTerm = q->nextTerm();
	   int qryTermID = db->term(qryTerm->spelling());
	   qt[qryTermID] ++;
	   qryFreqSum ++;
	}

    // Call retreival Method
    retrieve(qt, *db,accumulator, results);

	// sort results based on caculated scores
    results.Sort();

    //delete(stopper);
    delete[]qChar;

    // reset out results page to initialize it...
    output->resetResultsPage();
    output->setResultQuery(*query);

    results=removeDuplicateResults(results);

    // Display results
    //
    if (results.size() == 0) {
      output->setResultStatistics(0, 0, 0, 0);
      output->displayResultsPageBeginning();
      output->outputString("No results.");
      output->displayResultsPageEnding();
    } else {
      displaySearchResults(db, datasourceID, q, &results, listLength, rankStart);
    } // end [else] if (results.size() == 0)

   // delete(model);
   // delete(qr);
    delete(q);

  delete (db);
}

void DBInterface::getParsedDoc(long docID) {
  Index *db=openIndex();
  if (!db) {
    output->writeErrorMessage("Cannot open index.", "Error opening index: " + pathToIndex);
    return;
  }

  TermInfoList* termlist = db->termInfoListSeq(docID);
  if (!termlist) {
    output->writeErrorMessage("Cannot find parsed document.", "Cannot find parsed document termlist.");
    delete db;
    return;
  }

  TermInfoList::iterator it;
  std::string doc = "";

  termlist->startIteration();
  while (termlist->hasMore()) {
    TermInfo *thisEntry=termlist->nextEntry();
    if (thisEntry) {
      doc+=db->term(thisEntry->termID()) + " ";
    }
  }

  // just print out the whole string
  output->displayDataPage(doc, "Parsed Document", false);

  delete termlist;

  // close and release the index...
  delete db;
}

void DBInterface::getDocIID(long docID) {
  Index *db=openIndex();
  if (!db) {
    output->writeErrorMessage("Cannot open index.", "Error opening index: " + pathToIndex);
    return;
  }

  const DocumentManager* dm = NULL;
  string idstr;
  stringstream docidStrStr;
  docidStrStr << docID;

  if (! (dm = db->docManager (docID))) {
    output->writeErrorMessage("No document manager found.", "Could got retrieve document manager for document: " + docidStrStr.str());
    delete db;
    return;
  }

  idstr = db->document (docID);

  if (idstr == "[OOV]") {
    output->writeErrorMessage("Document not found.", "Document not found. Document: " + docidStrStr.str());
  } else {
    output->displayDataPage(dm->getDoc(idstr), "Document by Internal ID");
  }

  delete db;
}

void DBInterface::getDocXID(string *docID) {
  Index *db=openIndex();
  if (!db) {
    output->writeErrorMessage("Cannot open index.", "Error opening index: " + pathToIndex);
    return;
  }

  const DocumentManager* dm = NULL;
  if (docID) {
    int internalDocID=db->document(docID->c_str());
    if (internalDocID) {
      dm=db->docManager(internalDocID);
      if (dm) {
        string s(dm->getDoc(docID->c_str()));
    	output->displayDataPage(s, "Document by External ID");
        delete db;
        return;
      }
    }
  }

  output->writeErrorMessage("Error retrieving document.", "Cannot retrieve document.");

  delete db;
}




void DBInterface::getWordStem(string *term) {
  Index *db=openIndex();
  if (!db) {
    output->writeErrorMessage("Cannot open index.", "Error opening index: " + pathToIndex);
    return;
  }

  // get stemmer
  Stemmer* stemmer = getDbStemmer(db);

  // get the word and stem it (if there's a stemmer...)
  char *word;
  if (stemmer) {
    word=stemmer->stemWord((char*)term->c_str());
    delete stemmer;
  } else {
    word=(char*)(term->c_str());
  }

  TERMID_T id = db->term(word);

  if (id == 0) {
    output->displayDataPage("[OOV]\n", "Word stem for " + (*term));
  } else {
    output->displayDataPage(word, "Word stem for " + (*term));
  }

  delete db;
}

