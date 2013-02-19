#include <iostream>

using namespace std; 

const std::string host  = "81.19.81.199"; // IP of overpass.osm.rambler.ru
const int port = 80;

int connect_and_save(string query, FILE *fp);
string createQuery(double lat, double lon);
