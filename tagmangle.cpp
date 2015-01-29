/*	tagmangle
	Richard Fairhurst, January 2015

	Syntax:	tagmangle tag infile replacefile outfile
	e.g.	./tagmangle highway oxfordshire-latest.osm.pbf example.txt output.osm.pbf
*/

#include <iostream>
#include <fstream>
#include <vector>
#include "osmformat.pb.h"

using namespace std;
#include "helpers.cpp"
#include "pbf_blocks.cpp"

int main(int argc, char* argv[]) {

	string replace_key = argv[1];

	// Open replacements list
	map<int, string> replace_list;
	fstream replacefile(argv[3], ios::in);
	if (!replacefile) { cerr << "Couldn't open replacements file." << endl; return -1; }
	string line;
	while (getline(replacefile,line)) {
		string id =line.substr(0, line.find(" "));
		string val=line.substr(line.find(" ")+1);
		replace_list[atoi(id.c_str())] = val;
	}
	replacefile.close();

	// Open .pbf
	fstream infile(argv[2], ios::in | ios::binary);
	fstream outfile(argv[4], ios::out | ios::binary);
	if (!infile) { cerr << "Couldn't open .pbf input file." << endl; return -1; }
	if (!outfile) { cerr << "Couldn't open .pbf output file." << endl; return -1; }

	HeaderBlock block;
	readBlock(&block, &infile);
	writeBlock(&block, &outfile, "OSMHeader");

	PrimitiveBlock pb;
	Way way;
	vector<string> strings(0);
	PrimitiveGroup pg;
	uint ct=0;
	while (!infile.eof()) {
		readBlock(&pb, &infile);
		readStringTable(&strings, &pb);
		map<string, int> tagMap;
		readStringMap(&tagMap, &pb);
		uint replaceIndex=findStringInTable(&replace_key, &tagMap, &pb);
		cout << "Block " << ct << " size " << pb.primitivegroup_size() << endl;
		ct++;

		// Read ways
		for (uint i=0; i<pb.primitivegroup_size(); i++) {
			pg = pb.primitivegroup(i);
			if (pg.ways_size()>0) {
				for (uint j=0; j<pg.ways_size(); j++) {
					way = pg.ways(j);
					if (replace_list.find(way.id()) != replace_list.end()) {
						setTag(&way, &tagMap, replaceIndex, replace_list[way.id()], &pb);
//						map<string, string> tags = getTags(&strings, &way);
//						cout << way.id() << " " << tags[replace_key] << endl;
//						cout << "-- replace with " << replace_list[way.id()] << endl;
					}
				}
			}
		}
		// write out (altered?) block
		writeBlock(&pb, &outfile, "OSMData");
	}

	// Close Protobuf and file handles
	google::protobuf::ShutdownProtobufLibrary();
	infile.close();
	outfile.close();
}

