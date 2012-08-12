#include "FugaHelperFuncs.h"
#include <QMessageBox>

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

/* convert string to quint16
 * @param string: string to convert
 */
quint16 string2quint16 (string s) {
    stringstream ss(s);
    quint16 i;
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

// show popup error
void showError (string in_msg) {
    cout << "showError: " << in_msg << endl;
    stringstream ss("");
    ss << "Error: " << in_msg;
    QMessageBox msgBox;
    msgBox.setText(ss.str().c_str());
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}

// trim whitespaces
std::string trim(std::string& pString,const std::string& pWhitespace)
{
    const size_t beginStr = pString.find_first_not_of(pWhitespace);
    if (beginStr == std::string::npos)
    {
        // no content
        return "";
    }

    const size_t endStr = pString.find_last_not_of(pWhitespace);
    const size_t range = endStr - beginStr + 1;

    return pString.substr(beginStr, range);
}

// get network from name
std::string name2network(std::string in_name) {
    if (in_name.empty()) return "";
    vector<string> splitted = split(in_name,"%");
    if (splitted.size() != 2) return "";
    return splitted[1];
}
