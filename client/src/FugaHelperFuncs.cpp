#include "FugaHelperFuncs.h"

using namespace std;

/* split string by "delim"
 * @param string: string to split
 * @param char: delimiter
 */
vector<string> split(string s, string delim) {
    vector<string> elems;
    stringstream ss("");
    ss << s;
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
