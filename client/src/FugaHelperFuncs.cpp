#include "FugaHelperFuncs.h"

using namespace std;

/* split string by "delim"
 * @param string: string to split
 * @param char: delimiter
 */
vector<string> split(string s, string delim) {
    vector<string> elems;
    stringstream ss(s);
    string item;
    char delimiter = *delim.data();
    while(std::getline(ss, item, delimiter)) {
        elems.push_back(item);
    }
    return elems;
}

/* replace strings in string
 */
string replace(string str, string in, string out) {

    // find first
    unsigned int position = str.find(in);

    // replace all
    while ( position != string::npos ) {
        str.replace(position, 1, out);
        position = str.find(in, (position + 1));
    }

    return str;
}

/* join to char-arrays
 * @param string: first array to merge
 * @param string: next array to merge
 */
string merge (string array1, string array2) {
    string s;
    ostringstream ss(s);
    ss << array1;
    ss << array2;
    return s;
}

/* convert string to int
 * @param string: string to convert
 */
int string2int (string s) {
    stringstream ss(s);
    int i;
    if( (ss >> i).fail() ) {
        // conversion failed
        return 0;
    }
    return i;
}

/* convert string to int
 * @param string: string to convert
 */
int char2int (char* s) {
    stringstream ss(s);
    int i;
    if( (ss >> i).fail() ) {
        // conversion failed
        return 0;
    }
    return i;
}

/* convert int to string
 * @param int: int to convert
 */
string int2string (int i) {
    string s;
    stringstream ss(s);
    if( (ss << i).fail() ) {
        // conversion failed
        return "";
    }
    return s;
}
