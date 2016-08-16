#pragma once

#include <iostream>
#include <stdlib.h>
#include <string>
//#include <fstream>
#include <wchar.h>
using namespace std;

#define COMMON_OPT 	1
#define COMMAND_OPT 	2
#define FILE_OPT 	3
#define COMMON_FLAG 	4
#define COMMAND_FLAG 	5
#define FILE_FLAG 	6

#define COMMAND_OPTION_TYPE  	1
#define COMMAND_FLAG_TYPE 	2
#define FILE_OPTION_TYPE  	3
#define FILE_FLAG_TYPE 		4 
#define UNKNOWN_TYPE 		5

#define DEFAULT_MAXOPTS 	10
#define MAX_LONG_PREFIX_LENGTH 	2

#define DEFAULT_MAXUSAGE	3
#define DEFAULT_MAXHELP         10	

#define TRUE_FLAG L"true" 

using namespace std;

class AnyOption
{

public: /* the public interface */
    AnyOption();
    AnyOption(int maxoptions);
    AnyOption(int maxoptions, int maxcharoptions);
    ~AnyOption();

    /*
    * following set methods specifies the
    * special characters and delimiters
    * if not set traditional defaults will be used
    */

    void setCommandPrefixChar(wchar_t _prefix);   /* '-' in "-w" */
    void setCommandLongPrefix(wchar_t *_prefix);  /* '--' in "--width" */
    void setFileCommentChar(wchar_t _comment);    /* '#' in shellscripts */
    void setFileDelimiterChar(wchar_t _delimiter);/* ':' in "width : 100" */

    /*
    * provide the input for the options
    * like argv[] for commndline and the
    * option file name  to use;
    */

    void useCommandArgs(int _argc, wchar_t **_argv);
    void useFiileName(const wchar_t *_filename);

    /*
    * turn off the POSIX style options
    * this means anything starting with a '-' or "--"
    * will be considered a valid option
    * which alo means you cannot add a bunch of
    * POIX options chars together like "-lr"  for "-l -r"
    *
    */

    void noPOSIX();

    /*
    * prints warning verbose if you set anything wrong
    */
    void setVerbose();


    /*
    * there are two types of options
    *
    * Option - has an associated value ( -w 100 )
    * Flag  - no value, just a boolean flag  ( -nogui )
    *
    * the options can be either a string ( GNU style )
    * or a character ( traditional POSIX style )
    * or both ( --width, -w )
    *
    * the options can be common to the commandline and
    * the optionfile, or can belong only to either of
    * commandline and optionfile
    *
    * following set methods, handle all the aboove
    * cases of options.
    */

    /* options comman to command line and option file */
    void setOption(const wchar_t *opt_string);
    void setOption(wchar_t  opt_char);
    void setOption(const wchar_t *opt_string, wchar_t opt_char);
    void setFlag(const wchar_t *opt_string);
    void setFlag(wchar_t  opt_char);
    void setFlag(const wchar_t *opt_string, wchar_t opt_char);

    /* options read from commandline only */
    void setCommandOption(const wchar_t *opt_string);
    void setCommandOption(wchar_t  opt_char);
    void setCommandOption(const wchar_t *opt_string, wchar_t opt_char);
    void setCommandFlag(const wchar_t *opt_string);
    void setCommandFlag(wchar_t  opt_char);
    void setCommandFlag(const wchar_t *opt_string, wchar_t opt_char);

    /* options read from an option file only  */
    void setFileOption(const wchar_t *opt_string);
    void setFileOption(wchar_t  opt_char);
    void setFileOption(const wchar_t *opt_string, wchar_t opt_char);
    void setFileFlag(const wchar_t *opt_string);
    void setFileFlag(wchar_t  opt_char);
    void setFileFlag(const wchar_t *opt_string, wchar_t opt_char);

    /*
    * process the options, registerd using
    * useCommandArgs() and useFileName();
    */
    void processOptions();
    void processCommandArgs();
    void processCommandArgs(int max_args);
    bool processFile();

    /*
    * process the specified options
    */
    void processCommandArgs(int _argc, wchar_t **_argv);
    void processCommandArgs(int _argc, wchar_t **_argv, int max_args);
    //bool processFile( const wchar_t *_filename );

    /*
    * get the value of the options
    * will return NULL if no value is set
    */
    wchar_t *getValue(const wchar_t *_option);
    bool  getFlag(const wchar_t *_option);
    wchar_t *getValue(wchar_t _optchar);
    bool  getFlag(wchar_t _optchar);

    /*
    * Print Usage
    */
    void printUsage();
    void printAutoUsage();
    void addUsage(const wchar_t *line);
    void printHelp();
    /* print auto usage printing for unknown options or flag */
    void autoUsagePrint(bool flag);

    /*
    * get the argument count and arguments sans the options
    */
    int   getArgc();
    wchar_t* getArgv(int index);
    bool  hasOptions();

private: /* the hidden data structure */
    int argc;		/* commandline arg count  */
    wchar_t **argv;  		/* commndline args */
    const wchar_t* filename; 	/* the option file */
    wchar_t* appname; 	/* the application name from argv[0] */

    int *new_argv; 		/* arguments sans options (index to argv) */
    int new_argc;   	/* argument count sans the options */
    int max_legal_args; 	/* ignore extra arguments */


    /* option strings storage + indexing */
    int max_options; 	/* maximum number of options */
    const wchar_t **options; 	/* storage */
    int *optiontype; 	/* type - common, command, file */
    int *optionindex;	/* index into value storage */
    int option_counter; 	/* counter for added options  */

    /* option chars storage + indexing */
    int max_char_options; 	/* maximum number options */
    wchar_t *optionchars; 	/*  storage */
    int *optchartype; 	/* type - common, command, file */
    int *optcharindex; 	/* index into value storage */
    int optchar_counter; 	/* counter for added options  */

    /* values */
    wchar_t **values; 		/* common value storage */
    int g_value_counter; 	/* globally updated value index LAME! */

    /* help and usage */
    const wchar_t **usage; 	/* usage */
    int max_usage_lines;	/* max usage lines reseverd */
    int usage_lines;	/* number of usage lines */

    bool command_set;	/* if argc/argv were provided */
    bool file_set;		/* if a filename was provided */
    bool mem_allocated;     /* if memory allocated in init() */
    bool posix_style; 	/* enables to turn off POSIX style options */
    bool verbose;		/* silent|verbose */
    bool print_usage;	/* usage verbose */
    bool print_help;	/* help verbose */

    wchar_t opt_prefix_char;		/*  '-' in "-w" */
    wchar_t long_opt_prefix[MAX_LONG_PREFIX_LENGTH]; /* '--' in "--width" */
    wchar_t file_delimiter_char;	/* ':' in width : 100 */
    wchar_t file_comment_char;		/*  '#' in "#this is a comment" */
    wchar_t equalsign;
    wchar_t comment;
    wchar_t delimiter;
    wchar_t endofline;
    wchar_t whitespace;
    wchar_t nullterminate;

    bool set;   //was static member
    bool once;  //was static member

    bool hasoptions;
    bool autousage;

private: /* the hidden utils */
    void init();
    void init(int maxopt, int maxcharopt);
    bool alloc();
    void cleanup();
    bool valueStoreOK();

    /* grow storage arrays as required */
    bool doubleOptStorage();
    bool doubleCharStorage();
    bool doubleUsageStorage();

    bool setValue(const wchar_t *option, wchar_t *value);
    bool setFlagOn(const wchar_t *option);
    bool setValue(wchar_t optchar, wchar_t *value);
    bool setFlagOn(wchar_t optchar);

    void addOption(const wchar_t* option, int type);
    void addOption(wchar_t optchar, int type);
    void addOptionError(const wchar_t *opt);
    void addOptionError(wchar_t opt);
    bool findFlag(wchar_t* value);
    void addUsageError(const wchar_t *line);
    bool CommandSet();
    bool FileSet();
    bool POSIX();

    wchar_t parsePOSIX(wchar_t* arg);
    int parseGNU(wchar_t *arg);
    bool matchChar(wchar_t c);
    int matchOpt(wchar_t *opt);

    /* dot file methods */
    wchar_t *readFile();
    wchar_t *readFile(const wchar_t* fname);
    bool consumeFile(char *buffer);
    void processLine(wchar_t *theline, int length);
    wchar_t *chomp(wchar_t *str);
    void valuePairs(wchar_t *type, wchar_t *value);
    void justValue(wchar_t *value);

    void printVerbose(const wchar_t *msg);
    void printVerbose(wchar_t *msg);
    void printVerbose(wchar_t ch);
    void printVerbose();


};
