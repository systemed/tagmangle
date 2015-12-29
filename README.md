# tagmangle

A simple C++ utility to insert (or replace) way tags in an .osm.pbf file according to a supplied plain-text file.

In other words, you can create a text file which says way 9789873's highway tag should be 'residential', way 9789874 should be 'unclassified', and so on. Run tagmangle with this file and a pbf, and the result will be a new pbf in which those values have been set.

You can apply several text files to one pbf in the same run, if you have multiple keys to change.

## Compiling

You'll need the protobuf library. On OS X you can install this with Homebrew with `brew install protobuf`.

You can then compile tagmangle like this:

     clang++ -o tagmangle osmformat.pb.cc tagmangle.cpp -std=c++11 -lz `pkg-config --cflags --libs protobuf`

## Running

Create a text file with each way ID and its value on a new line, separated by a space. See example.txt for an example.

Run tagmangle with this syntax:

    tagmangle input_pbf output_pbf tag_key_1 text_file_1 [tag_key_2 text_file_2 ...]

For example:

    ./tagmangle oxfordshire-latest.osm.pbf output.osm.pbf highway example.txt

## Notes

This is my first ever C++ program. It is doubtless horrible in many many ways. Be gentle.

It doesn't check whether the PBF block size would be exceeded. In practice this is unlikely to be a problem.

The PBF handling code in pbf_blocks.cpp might be useful as an absolute bare-bones PBF parser for similar projects.

You don't need osmformat.proto but it's included in case you want to recompile using protoc. Note that BlobHeader and Blob have been added to the commonly-used version.

Richard Fairhurst, January 2015
