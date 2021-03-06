#ifndef _LEMUR_SEARCH_CGI_CONSTANTS_H
#define _LEMUR_SEARCH_CGI_CONSTANTS_H

/**
 * Structure to hold our index items
 */
typedef struct {
  /** the path to the index */
  string path;
  /** the description of the index */
  string name;
} db_t;

/** 
 * Default path to the configuration file
 */
#define LEMURCGI_CONFIG_PATH "./lemur.config"

/*
 * Default number of search result items per page to display
 */
#define DEFAULT_RESULTS_PER_PAGE 50

/**
 * Defines the maximum documents to retireve total.
 * Set to 0 for no limit.
 */
#define DEFAULT_MAX_DOCUMENTS_TO_RETRIEVE 0

/**
 * Defines the default path to the HTML templates
 */
#define DEFAULT_TEMPLATE_PATH "./templates/"

/**
 * LEMUR CGI version and date
 */
#define LEMUR_CGI_VERSION_NUMBER  "3.1"
#define LEMUR_CGI_COMPILE_DATE    __DATE__

#endif // _LEMUR_SEARCH_CGI_CONSTANTS_H

