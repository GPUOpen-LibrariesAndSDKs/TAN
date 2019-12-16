/*
The MIT License (MIT)

Copyright (c) 2016 Kishan Thomas www.hackorama.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

* AnyOption 1.3
*
* kishan at hackorama dot com  www.hackorama.com JULY 2001
*
* + Acts as a common facade class for reading
*   commandline options as well as options from
*   an optionfile with delimited type value pairs
*
* + Handles the POSIX style single character options ( -w )
*   as well as the newer GNU long options ( --width )
*
* + The option file assumes the traditional format of
*   first character based comment lines and type value
*   pairs with a delimiter , and flags which are not pairs
*
*      # this is a coment
*      # next line is an option value pair
*      width : 100
*         # next line is a flag
*      noimages
*
* + Supports printing out Help and Usage
*
* + Why not just use getopt() ?
*
*   getopt() Its a POSIX standard not part of ANSI-C.
*   So it may not be available on platforms like Windows.
*
* + Why it is so long ?
*
*   The actual code which does command line parsing
*   and option file parsing are done in  few methods.
*   Most of the extra code are for providing a flexible
*   common public interface to both a resourcefile and
*   and command line supporting POSIX style and
*   GNU long option as well as mixing of both.
*
* + Please see "anyoption.h" for public method descriptions
*
*/

/* Updated Auguest 2004
* Fix from  Michael D Peters (mpeters at sandia.gov)
* to remove static local variables, allowing multiple instantiations
* of the reader (for using multiple configuration files).  There is
* an error in the destructor when using multiple instances, so you
* cannot delete your objects (it will crash), but not calling the
* destructor only introduces a small memory leak, so I
* have not bothered tracking it down.
*
* Also updated to use modern C++ style headers, rather than
* depricated iostream.h (it was causing my compiler problems)
*/

/*
* Updated September 2006
* Fix from Boyan Asenov for a bug in mixing up option indexes
* leading to exception when mixing different options types
*/

#include "anyOption.h"

#pragma warning(disable: 4996)

AnyOption::AnyOption()
{
    init();
}

AnyOption::AnyOption(int maxopt)
{
    init(maxopt, maxopt);
}

AnyOption::AnyOption(int maxopt, int maxcharopt)
{
    init(maxopt, maxcharopt);
}

AnyOption::~AnyOption()
{
    if (mem_allocated)
        cleanup();
}

void
AnyOption::init()
{
    init(DEFAULT_MAXOPTS, DEFAULT_MAXOPTS);
}

void
AnyOption::init(int maxopt, int maxcharopt)
{

    max_options = maxopt;
    max_char_options = maxcharopt;
    max_usage_lines = DEFAULT_MAXUSAGE;
    usage_lines = 0;
    argc = 0;
    argv = NULL;
    posix_style = true;
    verbose = false;
    filename = NULL;
    appname = NULL;
    option_counter = 0;
    optchar_counter = 0;
    new_argv = NULL;
    new_argc = 0;
    max_legal_args = 0;
    command_set = false;
    file_set = false;
    values = NULL;
    g_value_counter = 0;
    mem_allocated = false;
    command_set = false;
    file_set = false;
    opt_prefix_char = '-';
    file_delimiter_char = ':';
    file_comment_char = '#';
    equalsign = '=';
    comment = '#';
    delimiter = ':';
    endofline = '\n';
    whitespace = ' ';
    nullterminate = '\0';
    set = false;
    once = true;
    hasoptions = false;
    autousage = false;

    wcscpy(long_opt_prefix, L"--");

    if (alloc() == false){
        cout << endl << "OPTIONS ERROR : Failed allocating memory";
        cout << endl;
        cout << "Exiting." << endl;
        exit(0);
    }
}

bool
AnyOption::alloc()
{
    int i = 0;
    int size = 0;

    if (mem_allocated)
        return true;

    size = (max_options + 1) * sizeof(const wchar_t*);
    options = (const wchar_t**)malloc(size);    //
    optiontype = (int*)malloc((max_options + 1)*sizeof(int));
    optionindex = (int*)malloc((max_options + 1)*sizeof(int));
    if (options == NULL || optiontype == NULL || optionindex == NULL)
        return false;
    else
        mem_allocated = true;
    for (i = 0; i < max_options; i++){
        options[i] = NULL;
        optiontype[i] = 0;
        optionindex[i] = -1;
    }
    optionchars = (wchar_t*)malloc((max_char_options + 1)*sizeof(wchar_t));
    optchartype = (int*)malloc((max_char_options + 1)*sizeof(int));
    optcharindex = (int*)malloc((max_char_options + 1)*sizeof(int));
    if (optionchars == NULL ||
        optchartype == NULL ||
        optcharindex == NULL)
    {
        mem_allocated = false;
        return false;
    }
    for (i = 0; i < max_char_options; i++){
        optionchars[i] = '0';
        optchartype[i] = 0;
        optcharindex[i] = -1;
    }

    size = (max_usage_lines + 1) * sizeof(const wchar_t*);
    usage = (const wchar_t**)malloc(size);

    if (usage == NULL){
        mem_allocated = false;
        return false;
    }
    for (i = 0; i < max_usage_lines; i++)
        usage[i] = NULL;

    return true;
}

bool
AnyOption::doubleOptStorage()
{
    options = (const wchar_t**)realloc(options,
        ((2 * max_options) + 1) * sizeof(const wchar_t*));
    optiontype = (int*)realloc(optiontype,
        ((2 * max_options) + 1)* sizeof(int));
    optionindex = (int*)realloc(optionindex,
        ((2 * max_options) + 1) * sizeof(int));
    if (options == NULL || optiontype == NULL || optionindex == NULL)
        return false;
    /* init new storage */
    for (int i = max_options; i < 2 * max_options; i++){
        options[i] = NULL;
        optiontype[i] = 0;
        optionindex[i] = -1;
    }
    max_options = 2 * max_options;
    return true;
}

bool
AnyOption::doubleCharStorage()
{
    optionchars = (wchar_t*)realloc(optionchars,
        ((2 * max_char_options) + 1)*sizeof(wchar_t));
    optchartype = (int*)realloc(optchartype,
        ((2 * max_char_options) + 1)*sizeof(int));
    optcharindex = (int*)realloc(optcharindex,
        ((2 * max_char_options) + 1)*sizeof(int));
    if (optionchars == NULL ||
        optchartype == NULL ||
        optcharindex == NULL)
        return false;
    /* init new storage */
    for (int i = max_char_options; i < 2 * max_char_options; i++){
        optionchars[i] = '0';
        optchartype[i] = 0;
        optcharindex[i] = -1;
    }
    max_char_options = 2 * max_char_options;
    return true;
}

bool
AnyOption::doubleUsageStorage()
{
    usage = (const wchar_t**)realloc(usage,
        ((2 * max_usage_lines) + 1) * sizeof(const wchar_t*));
    if (usage == NULL)
        return false;
    for (int i = max_usage_lines; i < 2 * max_usage_lines; i++)
        usage[i] = NULL;
    max_usage_lines = 2 * max_usage_lines;
    return true;

}


void
AnyOption::cleanup()
{
    free(options);
    free(optiontype);
    free(optionindex);
    free(optionchars);
    free(optchartype);
    free(optcharindex);
    free(usage);

    if (values != NULL)
    {
        for (int i = 0; i < g_value_counter; i++)
        {
            if (values[i] != NULL)
            {
                free(values[i]);
                values[i] = NULL;
            }
        }
        free(values);
    }
    if (new_argv != NULL)
        free(new_argv);
}

void
AnyOption::setCommandPrefixChar(wchar_t _prefix)
{
    opt_prefix_char = _prefix;
}

void
AnyOption::setCommandLongPrefix(wchar_t *_prefix)
{
    if (wcslen(_prefix) > MAX_LONG_PREFIX_LENGTH){
        *(_prefix + MAX_LONG_PREFIX_LENGTH) = '\0';
    }

    wcscpy(long_opt_prefix, _prefix);
}

void
AnyOption::setFileCommentChar(wchar_t _comment)
{
    file_delimiter_char = _comment;
}


void
AnyOption::setFileDelimiterChar(wchar_t _delimiter)
{
    file_comment_char = _delimiter;
}

bool
AnyOption::CommandSet()
{
    return(command_set);
}

bool
AnyOption::FileSet()
{
    return(file_set);
}

void
AnyOption::noPOSIX()
{
    posix_style = false;
}

bool
AnyOption::POSIX()
{
    return posix_style;
}


void
AnyOption::setVerbose()
{
    verbose = true;
}

void
AnyOption::printVerbose()
{
    if (verbose)
        cout << endl;
}
void
AnyOption::printVerbose(const wchar_t *msg)
{
    if (verbose)
        cout << msg;
}

void
AnyOption::printVerbose(wchar_t *msg)
{
    if (verbose)
        cout << msg;
}

void
AnyOption::printVerbose(wchar_t ch)
{
    if (verbose)
        cout << ch;
}

bool
AnyOption::hasOptions()
{
    return hasoptions;
}

void
AnyOption::autoUsagePrint(bool _autousage)
{
    autousage = _autousage;
}

void
AnyOption::useCommandArgs(int _argc, wchar_t **_argv)
{
    argc = _argc;
    argv = _argv;
    command_set = true;
    appname = argv[0];
    if (argc > 1) hasoptions = true;
}

void
AnyOption::useFiileName(const wchar_t *_filename)
{
    filename = _filename;
    file_set = true;
}

/*
* set methods for options
*/

void
AnyOption::setCommandOption(const wchar_t *opt)
{
    addOption(opt, COMMAND_OPT);
    g_value_counter++;
}

void
AnyOption::setCommandOption(wchar_t opt)
{
    addOption(opt, COMMAND_OPT);
    g_value_counter++;
}

void
AnyOption::setCommandOption(const wchar_t *opt, wchar_t optchar)
{
    addOption(opt, COMMAND_OPT);
    addOption(optchar, COMMAND_OPT);
    g_value_counter++;
}

void
AnyOption::setCommandFlag(const wchar_t *opt)
{
    addOption(opt, COMMAND_FLAG);
    g_value_counter++;
}

void
AnyOption::setCommandFlag(wchar_t opt)
{
    addOption(opt, COMMAND_FLAG);
    g_value_counter++;
}

void
AnyOption::setCommandFlag(const wchar_t *opt, wchar_t optchar)
{
    addOption(opt, COMMAND_FLAG);
    addOption(optchar, COMMAND_FLAG);
    g_value_counter++;
}

void
AnyOption::setFileOption(const wchar_t *opt)
{
    addOption(opt, FILE_OPT);
    g_value_counter++;
}

void
AnyOption::setFileOption(wchar_t opt)
{
    addOption(opt, FILE_OPT);
    g_value_counter++;
}

void
AnyOption::setFileOption(const wchar_t *opt, wchar_t optchar)
{
    addOption(opt, FILE_OPT);
    addOption(optchar, FILE_OPT);
    g_value_counter++;
}

void
AnyOption::setFileFlag(const wchar_t *opt)
{
    addOption(opt, FILE_FLAG);
    g_value_counter++;
}

void
AnyOption::setFileFlag(wchar_t opt)
{
    addOption(opt, FILE_FLAG);
    g_value_counter++;
}

void
AnyOption::setFileFlag(const wchar_t *opt, wchar_t optchar)
{
    addOption(opt, FILE_FLAG);
    addOption(optchar, FILE_FLAG);
    g_value_counter++;
}

void
AnyOption::setOption(const wchar_t *opt)
{
    addOption(opt, COMMON_OPT);
    g_value_counter++;
}

void
AnyOption::setOption(wchar_t opt)
{
    addOption(opt, COMMON_OPT);
    g_value_counter++;
}

void
AnyOption::setOption(const wchar_t *opt, wchar_t optchar)
{
    addOption(opt, COMMON_OPT);
    addOption(optchar, COMMON_OPT);
    g_value_counter++;
}

void
AnyOption::setFlag(const wchar_t *opt)
{
    addOption(opt, COMMON_FLAG);
    g_value_counter++;
}

void
AnyOption::setFlag(const wchar_t opt)
{
    addOption(opt, COMMON_FLAG);
    g_value_counter++;
}

void
AnyOption::setFlag(const wchar_t *opt, wchar_t optchar)
{
    addOption(opt, COMMON_FLAG);
    addOption(optchar, COMMON_FLAG);
    g_value_counter++;
}

void //here **wchar_t problem
AnyOption::addOption(const wchar_t *opt, int type)
{
    if (option_counter >= max_options){
        if (doubleOptStorage() == false){
            addOptionError(opt);
            return;
        }
    }
    options[option_counter] = opt;
    optiontype[option_counter] = type;
    optionindex[option_counter] = g_value_counter;
    option_counter++;
}

void
AnyOption::addOption(wchar_t opt, int type)
{
    if (!POSIX()){
        printVerbose(L"Ignoring the option character \"");
        printVerbose(opt);
        printVerbose(L"\" ( POSIX options are turned off )");
        printVerbose();
        return;
    }


    if (optchar_counter >= max_char_options){
        if (doubleCharStorage() == false){
            addOptionError(opt);
            return;
        }
    }
    optionchars[optchar_counter] = opt;
    optchartype[optchar_counter] = type;
    optcharindex[optchar_counter] = g_value_counter;
    optchar_counter++;

}

void
AnyOption::addOptionError(const wchar_t *opt)
{
    cout << endl;
    cout << "OPTIONS ERROR : Failed allocating extra memory " << endl;
    cout << "While adding the option : \"" << opt << "\"" << endl;
    cout << "Exiting." << endl;
    cout << endl;
    exit(0);
}

void
AnyOption::addOptionError(wchar_t opt)
{
    cout << endl;
    cout << "OPTIONS ERROR : Failed allocating extra memory " << endl;
    cout << "While adding the option: \"" << opt << "\"" << endl;
    cout << "Exiting." << endl;
    cout << endl;
    exit(0);
}

void
AnyOption::processOptions()
{
    if (!valueStoreOK())
        return;
}

void
AnyOption::processCommandArgs(int max_args)
{
    max_legal_args = max_args;
    processCommandArgs();
}

void
AnyOption::processCommandArgs(int _argc, wchar_t **_argv, int max_args)
{
    max_legal_args = max_args;
    processCommandArgs(_argc, _argv);
}

void
AnyOption::processCommandArgs(int _argc, wchar_t **_argv)
{
    useCommandArgs(_argc, _argv);
    processCommandArgs();
}

void
AnyOption::processCommandArgs()
{
    if (!(valueStoreOK() && CommandSet()))
        return;

    if (max_legal_args == 0)
        max_legal_args = argc;
    new_argv = (int*)malloc((max_legal_args + 1) * sizeof(int));
    for (int i = 1; i < argc; i++){/* ignore first argv */
        if (argv[i][0] == long_opt_prefix[0] &&
            argv[i][1] == long_opt_prefix[1]) { /* long GNU option */
            int match_at = parseGNU(argv[i] + 2); /* skip -- */
            if (match_at >= 0 && i < argc - 1) /* found match */
                setValue(options[match_at], argv[++i]);
        }
        else if (argv[i][0] == opt_prefix_char) { /* POSIX char */
            if (POSIX()){
                wchar_t ch = parsePOSIX(argv[i] + 1);/* skip - */
                if (ch != '0' && i < argc - 1) /* matching char */
                    setValue(ch, argv[++i]);
            }
            else { /* treat it as GNU option with a - */
                int match_at = parseGNU(argv[i] + 1); /* skip - */
                if (match_at >= 0 && i < argc - 1) /* found match */
                    setValue(options[match_at], argv[++i]);
            }
        }
        else { /* not option but an argument keep index */
            if (new_argc < max_legal_args){
  
                // HACK TO ADD FLAG WITHOUT '-'
                parseGNU(argv[i]);

                // Original
                //new_argv[new_argc] = i;
                //new_argc++;
            }
            else{ /* ignore extra arguments */
                printVerbose(L"Ignoring extra argument: ");
                printVerbose(argv[i]);
                printVerbose();
                printAutoUsage();
            }
            printVerbose(L"Unknown command argument option : ");
            printVerbose(argv[i]);
            printVerbose();
            printAutoUsage();
        }
    }
}

wchar_t
AnyOption::parsePOSIX(wchar_t* arg)
{

    for (unsigned int i = 0; i < wcslen(arg); i++){
        wchar_t ch = arg[i];
        if (matchChar(ch)) { /* keep matching flags till an option */
            /*if last char argv[++i] is the value */
            if (i == wcslen(arg) - 1){
                return ch;
            }
            else{/* else the rest of arg is the value */
                i++; /* skip any '=' and ' ' */
                while (arg[i] == whitespace
                    || arg[i] == equalsign)
                    i++;
                setValue(ch, arg + i);
                return '0';
            }
        }
    }
    printVerbose(L"Unknown command argument option : ");
    printVerbose(arg);
    printVerbose();
    printAutoUsage();
    return '0';
}

int
AnyOption::parseGNU(wchar_t *arg)
{
    int split_at = 0;
    /* if has a '=' sign get value */
    for (unsigned int i = 0; i < wcslen(arg); i++){
        if (arg[i] == equalsign){
            split_at = i; /* store index */
            i = static_cast<unsigned int>(wcslen(arg)); /* get out of loop */
        }
    }
    if (split_at > 0){ /* it is an option value pair */
        wchar_t* tmp = (wchar_t*)malloc((split_at + 1)*sizeof(wchar_t));
        for (int i = 0; i < split_at; i++)
            tmp[i] = arg[i];
        tmp[split_at] = '\0';

        if (matchOpt(tmp) >= 0){
            setValue(options[matchOpt(tmp)], arg + split_at + 1);
            free(tmp);
        }
        else{
            printVerbose(L"Unknown command argument option : ");
            printVerbose(arg);
            printVerbose();
            printAutoUsage();
            free(tmp);
            return -1;
        }
    }
    else{ /* regular options with no '=' sign  */
        return  matchOpt(arg);
    }
    return -1;
}


int
AnyOption::matchOpt(wchar_t *opt)
{
    for (int i = 0; i < option_counter; i++){
        if (wcscmp(options[i], opt) == 0){
            if (optiontype[i] == COMMON_OPT ||
                optiontype[i] == COMMAND_OPT)
            { /* found option return index */
                return i;
            }
            else if (optiontype[i] == COMMON_FLAG ||
                optiontype[i] == COMMAND_FLAG)
            { /* found flag, set it */
                setFlagOn(opt);
                return -1;
            }
        }
    }
    printVerbose(L"Unknown command argument option : ");
    printVerbose(opt);
    printVerbose();
    printAutoUsage();
    return  -1;
}
bool
AnyOption::matchChar(wchar_t c)
{
    for (int i = 0; i < optchar_counter; i++){
        if (optionchars[i] == c) { /* found match */
            if (optchartype[i] == COMMON_OPT ||
                optchartype[i] == COMMAND_OPT)
            { /* an option store and stop scanning */
                return true;
            }
            else if (optchartype[i] == COMMON_FLAG ||
                optchartype[i] == COMMAND_FLAG) { /* a flag store and keep scanning */
                setFlagOn(c);
                return false;
            }
        }
    }
    printVerbose(L"Unknown command argument option : ");
    printVerbose(c);
    printVerbose();
    printAutoUsage();
    return false;
}

bool
AnyOption::valueStoreOK()
{
    int size = 0;
    if (!set){
        if (g_value_counter > 0){
            size = g_value_counter * sizeof(wchar_t*);
            values = (wchar_t**)malloc(size);
            for (int i = 0; i < g_value_counter; i++)
                values[i] = NULL;
            set = true;
        }
    }
    return  set;
}

/*
* public get methods
*/
wchar_t*
AnyOption::getValue(const wchar_t *option)
{
    if (!valueStoreOK())
        return NULL;

    for (int i = 0; i < option_counter; i++){
        if (wcscmp(options[i], option) == 0)
            return values[optionindex[i]];
    }
    return NULL;
}

bool
AnyOption::getFlag(const wchar_t *option)
{
    if (!valueStoreOK())
        return false;
    for (int i = 0; i < option_counter; i++){
        if (wcscmp(options[i], option) == 0)
            return findFlag(values[optionindex[i]]);
    }
    return false;
}

wchar_t*
AnyOption::getValue(wchar_t option)
{
    if (!valueStoreOK())
        return NULL;
    for (int i = 0; i < optchar_counter; i++){
        if (optionchars[i] == option)
            return values[optcharindex[i]];
    }
    return NULL;
}

bool
AnyOption::getFlag(wchar_t option)
{
    if (!valueStoreOK())
        return false;
    for (int i = 0; i < optchar_counter; i++){
        if (optionchars[i] == option)
            return findFlag(values[optcharindex[i]]);
    }
    return false;
}

bool
AnyOption::findFlag(wchar_t* val)
{
    if (val == NULL)
        return false;

    if (wcscmp(TRUE_FLAG, val) == 0)
        return true;

    return false;
}

/*
* private set methods
*/
bool
AnyOption::setValue(const wchar_t *option, wchar_t *value)
{
    if (!valueStoreOK())
        return false;
    for (int i = 0; i < option_counter; i++){
        if (wcscmp(options[i], option) == 0){
            values[optionindex[i]] = (wchar_t*)malloc((wcslen(value) + 1)*sizeof(wchar_t));
            wcscpy(values[optionindex[i]], value);
            return true;
        }
    }
    return false;
}

bool
AnyOption::setFlagOn(const wchar_t *option)
{
    if (!valueStoreOK())
        return false;
    for (int i = 0; i < option_counter; i++){
        if (wcscmp(options[i], option) == 0){
            values[optionindex[i]] = (wchar_t*)malloc((wcslen(TRUE_FLAG) + 1)*sizeof(wchar_t));
            wcscpy(values[optionindex[i]], TRUE_FLAG);
            return true;
        }
    }
    return false;
}

bool
AnyOption::setValue(wchar_t option, wchar_t *value)
{
    if (!valueStoreOK())
        return false;
    for (int i = 0; i < optchar_counter; i++){
        if (optionchars[i] == option){
            values[optcharindex[i]] = (wchar_t*)malloc((wcslen(value) + 1)*sizeof(wchar_t));
            wcscpy(values[optcharindex[i]], value);
            return true;
        }
    }
    return false;
}

bool
AnyOption::setFlagOn(wchar_t option)
{
    if (!valueStoreOK())
        return false;
    for (int i = 0; i < optchar_counter; i++){
        if (optionchars[i] == option){
            values[optcharindex[i]] = (wchar_t*)malloc((wcslen(TRUE_FLAG) + 1)*sizeof(wchar_t));
            wcscpy(values[optcharindex[i]], TRUE_FLAG);
            return true;
        }
    }
    return false;
}


int
AnyOption::getArgc()
{
    return new_argc;
}

wchar_t*
AnyOption::getArgv(int index)
{
    if (index < new_argc){
        return (argv[new_argv[index]]);
    }
    return NULL;
}

/* dotfile sub routines */


/*->
bool
AnyOption::processFile()
{
if( ! (valueStoreOK() && FileSet())  )
return false;
return  ( consumeFile(readFile()) );
} <-*/

/*->
bool
AnyOption::processFile( const wchar_t *filename )
{
useFiileName(filename );
return ( processFile() );

}<-*/

/*
wchar_t*
AnyOption::readFile()
{
return ( readFile(filename) );
}<-*/

/*
* read the file contents to a character buffer
*/


/*->
wchar_t*
AnyOption::readFile( const wchar_t* fname )
{
int length;
wchar_t *buffer;
ifstream is;
is.open ( fname , ifstream::in );
if( ! is.good() ){
is.close();
return NULL;
}
is.seekg (0, ios::end);
length = is.tellg();
is.seekg (0, ios::beg);
buffer = (wchar_t*) malloc(length*sizeof(wchar_t));
is.read ((char*)buffer,length); //check
is.close();
return buffer;
}<-*/

/*
* scans a char* buffer for lines that does not
* start with the specified comment character.
*/
/*->
bool
AnyOption::consumeFile( char *buffer )
{

if( buffer == NULL )
return false;

wchar_t *cursor = buffer;// preserve the ptr
wchar_t *pline = NULL ;
int linelength = 0;
bool newline = true;
for( unsigned int i = 0 ; i < strlen( buffer ) ; i++ ){
if( *cursor == endofline ) { //end of line
if( pline != NULL ) //valid line
processLine( pline, linelength );
pline = NULL;
newline = true;
}else if( newline ){ //start of line
newline = false;
if( (*cursor != comment ) ){  //not a comment
pline = cursor ;
linelength = 0 ;
}
}
cursor++; //keep moving
linelength++;
}
free (buffer);
return true;
}<-*/


/*
*  find a valid type value pair separated by a delimiter
*  character and pass it to valuePairs()
*  any line which is not valid will be considered a value
*  and will get passed on to justValue()
*
*  assuming delimiter is ':' the behaviour will be,
*
*  width:10    - valid pair valuePairs( width, 10 );
*  width : 10  - valid pair valuepairs( width, 10 );
*
*  ::::        - not valid
*  width       - not valid
*  :10         - not valid
*  width:      - not valid
*  ::          - not valid
*  :           - not valid
*
*/

/*->
void
AnyOption::processLine( wchar_t *theline, int length  )
{
bool found = false;
wchar_t *pline = (wchar_t*) malloc( (length+1)*sizeof(wchar_t) );
for( int i = 0 ; i < length ; i ++ )
pline[i]= *(theline++);
pline[length] = nullterminate;
wchar_t *cursor = pline ; // preserve the ptr
if( *cursor == delimiter || *(cursor+length-1) == delimiter ){
justValue( pline );// line with start/end delimiter
}else{
for( int i = 1 ; i < length-1 && !found ; i++){//delimiter
if( *cursor == delimiter ){
*(cursor-1) = nullterminate; //wo strings
found = true;
valuePairs( pline , cursor+1 );
}
cursor++;
}
cursor++;
if( !found ) //not a pair
justValue( pline );
}
free (pline);
}<-*/

/*
* removes trailing and preceeding whitespaces from a string
*/
wchar_t*
AnyOption::chomp(wchar_t *str)
{
    while (*str == whitespace)
        str++;
    wchar_t *end = str + wcslen(str) - 1;
    while (*end == whitespace)
        end--;
    *(end + 1) = nullterminate;
    return str;
}

void
AnyOption::valuePairs(wchar_t *type, wchar_t *value)
{
    if (wcslen(chomp(type)) == 1){ /* this is a char option */
        for (int i = 0; i < optchar_counter; i++){
            if (optionchars[i] == type[0]){ /* match */
                if (optchartype[i] == COMMON_OPT ||
                    optchartype[i] == FILE_OPT)
                {
                    setValue(type[0], chomp(value));
                    return;
                }
            }
        }
    }
    /* if no char options matched */
    for (int i = 0; i < option_counter; i++){
        if (wcscmp(options[i], type) == 0){ /* match */
            if (optiontype[i] == COMMON_OPT ||
                optiontype[i] == FILE_OPT)
            {
                setValue(type, chomp(value));
                return;
            }
        }
    }
    printVerbose(L"Unknown option in resourcefile : ");
    printVerbose(type);
    printVerbose();
}

void
AnyOption::justValue(wchar_t *type)
{

    if (wcslen(chomp(type)) == 1){ /* this is a char option */
        for (int i = 0; i < optchar_counter; i++){
            if (optionchars[i] == type[0]){ /* match */
                if (optchartype[i] == COMMON_FLAG ||
                    optchartype[i] == FILE_FLAG)
                {
                    setFlagOn(type[0]);
                    return;
                }
            }
        }
    }
    /* if no char options matched */
    for (int i = 0; i < option_counter; i++){
        if (wcscmp(options[i], type) == 0){ /* match */
            if (optiontype[i] == COMMON_FLAG ||
                optiontype[i] == FILE_FLAG)
            {
                setFlagOn(type);
                return;
            }
        }
    }
    printVerbose(L"Unknown option in resourcefile : ");
    printVerbose(type);
    printVerbose();
}

/*
* usage and help
*/


void
AnyOption::printAutoUsage()
{
    if (autousage) printUsage();
}

void
AnyOption::printUsage()
{

    if (once) {
        once = false;
        wcout << endl;
        for (int i = 0; i < usage_lines; i++)
            wcout << usage[i] << endl;
        wcout << endl;
    }
}


void
AnyOption::addUsage(const wchar_t *line)
{
    if (usage_lines >= max_usage_lines){
        if (doubleUsageStorage() == false){
            addUsageError(line);
            exit(1);
        }
    }
    usage[usage_lines] = line;
    usage_lines++;
}

void
AnyOption::addUsageError(const wchar_t *line)
{
    cout << endl;
    cout << "OPTIONS ERROR : Failed allocating extra memory " << endl;
    cout << "While adding the usage/help  : \"" << line << "\"" << endl;
    cout << "Exiting." << endl;
    cout << endl;
    exit(0);

}