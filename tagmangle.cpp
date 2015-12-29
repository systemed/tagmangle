/*	tagmangle
	Richard Fairhurst, January 2015

	Syntax:	tagmangle infile outfile tag_1 replacefile_1 tag_2 replacefile_2...
	e.g. ./tagmangle oxfordshire-latest.osm.pbf output.osm.pbf highway example.txt

*/

#include <iostream>
#include <fstream>
#include <vector>
#include "osmformat.pb.h"

using namespace std;
#include "helpers.cpp"
#include "pbf_blocks.cpp"

int main(int argc, char* argv[]) {

	// Open .pbfs
	fstream infile(argv[1], ios::in | ios::binary);
	fstream outfile(argv[2], ios::out | ios::binary);
	if (!infile) { cerr << "Couldn't open .pbf input file." << endl; return -1; }
	if (!outfile) { cerr << "Couldn't open .pbf output file." << endl; return -1; }

	// Open replacement lists
	map<string, map<int, string> > replace_list;
	for (uint i=3; i<argc; i+=2) {
		string replace_key = argv[i];
		string replace_fn = argv[i+1];

		fstream replacefile(replace_fn, ios::in);
		if (!replacefile) { cerr << "Couldn't open replacements file " << replace_fn << endl; return -1; }
		cout << "Reading replacements for " << replace_key << " from " << replace_fn << endl;

		string line;
		while (getline(replacefile,line)) {
			string id =line.substr(0, line.find(" "));
			string val=line.substr(line.find(" ")+1);
			replace_list[replace_key][atoi(id.c_str())] = val;
		}
		replacefile.close();
	}

	// Copy pbf header
	HeaderBlock block;
	readBlock(&block, &infile);
	writeBlock(&block, &outfile, "OSMHeader");

	PrimitiveBlock pb;
	Way way;
	string key;
	vector<string> strings(0);
	PrimitiveGroup pg;
	uint ct=0;
	while (!infile.eof()) {
		readBlock(&pb, &infile);
		readStringTable(&strings, &pb);
		map<string, int> tagMap;
		readStringMap(&tagMap, &pb);

		cout << "Block " << ct << " size " << pb.primitivegroup_size() << "  \r";
		cout.flush();
		ct++;

		// Find position of each key in the string table
		map<string, uint> key_indexes;
		for (auto it=replace_list.begin(); it!=replace_list.end(); it++) {
			key = it->first;
			key_indexes[key] = findStringInTable(&key, &tagMap, &pb);
		}

		// Read ways
		for (uint i=0; i<pb.primitivegroup_size(); i++) {
			pg = pb.primitivegroup(i);
			if (pg.ways_size()>0) {
				for (uint j=0; j<pg.ways_size(); j++) {
					way = pg.ways(j);
					for (auto it=replace_list.begin(); it!=replace_list.end(); it++) {
						key = it->first;
						uint keyIndex = key_indexes[key];
						if (replace_list[key].find(way.id()) != replace_list[key].end()) {
							uint valueIndex = findStringInTable(&replace_list[key][way.id()], &tagMap, &pb);
							setTag(pb.mutable_primitivegroup(i)->mutable_ways(j), keyIndex, valueIndex);
						}
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
	cout << endl;
}
