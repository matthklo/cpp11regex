#include <iostream>
#include <regex>
#include <fstream>
#include <memory>
#include <cstring>
#include "xgetopt.h"

struct DataBuffer
{
	explicit DataBuffer(char *d, std::size_t sz)
	{
		data = d;
		len = sz;
	}

	virtual ~DataBuffer()
	{
		delete[] data;
	}

	static DataBuffer* buildFromFile(const std::string& filepath)
	{
		std::ifstream ifs(filepath);
		if (!ifs.is_open())
		{
			return nullptr;
		}

		std::streamoff offset = 
			(std::streamoff) ifs.seekg(0, std::ios_base::end).tellg();

		char *buf = new char[(unsigned int)offset];
		ifs.seekg(0, std::ios_base::beg);
		ifs.read(buf, offset);

		return new DataBuffer(buf, (std::size_t)offset);
	}

	static DataBuffer* buildFromCString(const char *rawdata)
	{
		auto len = std::strlen(rawdata);
		char* buf = new char[len];
		std::memcpy(buf, rawdata, len);

		return new DataBuffer(buf, len);
	}

	char* data;
	std::size_t len;
};

int
main(int argc, char** argv)
{
	std::string input_file;
	std::string regex_file;

	std::unique_ptr<DataBuffer> inbuf;
	std::unique_ptr<DataBuffer> regexbuf;
	std::regex_constants::syntax_option_type regex_syntax_type = std::regex_constants::ECMAScript;

#pragma region Parsing command line arguments
	while (true)
	{
		int ch = xgetopt(argc, argv, "i:r:g:");
		if (-1 == ch)
			break;

		switch (ch)
		{
		case 'i':
			input_file = xoptarg;
			break;

		case 'r':
			regex_file = xoptarg;
			break;

		case 'g': // "ECMAScript"(default), "basic", "extended", "awk", "grep", "egrep"
			{
				std::string gtype(xoptarg);
				if (gtype == "ECMAScript")
					regex_syntax_type = std::regex_constants::ECMAScript;
				else if (gtype == "basic")
					regex_syntax_type = std::regex_constants::basic;
				else if (gtype == "extended")
					regex_syntax_type = std::regex_constants::extended;
				else if (gtype == "awk")
					regex_syntax_type = std::regex_constants::awk;
				else if (gtype == "grep")
					regex_syntax_type = std::regex_constants::grep;
				else if (gtype == "egrep")
					regex_syntax_type = std::regex_constants::egrep;
			}
			break;

		case '?':
			break;

		default:
			break;
		}
	}

	if (input_file.empty())
	{
		fprintf(stderr, "Error: No input file.\n");
		return 1;
	}
#pragma endregion
	
#pragma region Preparing data
	inbuf.reset(DataBuffer::buildFromFile(input_file));

	if (nullptr == inbuf.get())
	{
		fprintf(stderr, "Error: Unable to open file: %s for reading.\n", input_file.c_str());
		return 1;
	}

	if (!regex_file.empty())
	{
		regexbuf.reset(DataBuffer::buildFromFile(regex_file));
		if (nullptr == regexbuf.get())
		{
			fprintf(stderr, "Error: Unable to open file: %s for reading.\n", regex_file.c_str());
			return 1;
		}
	}
	else if (xoptind < argc)
	{
		regexbuf.reset(DataBuffer::buildFromCString(argv[xoptind]));
	}
	else
	{
		fprintf(stderr, "Error: Neither regex input file nor regex pattern provided as a command line argument.\n");
		return 1;
	}
#pragma endregion

#pragma region Perform regex processing
	std::regex regexExpr(regexbuf->data, regexbuf->len, regex_syntax_type);

	std::cmatch results1;
	bool r = std::regex_match(
		(const char*)&inbuf->data[0], (const char*)&inbuf->data[inbuf->len], 
		results1, regexExpr);

	printf("std::regex_match\n");
	printf("    return value  : %s\n", r ? "true" : "false");
	printf("    match results : (%u)\n", results1.size());
	for (unsigned int idx = 0; idx < results1.size(); ++idx)
	{
		printf("        %u)\n", idx);
		printf("            %-7.7s: %u\n", "pos", results1.position(idx));
		printf("            %-7.7s: %u\n", "length", results1.length(idx));
		printf("            %-7.7s: %s\n", "str", results1.str(idx).c_str());
	}

	std::cmatch results2;
	r = std::regex_search(
		(const char*)&inbuf->data[0], (const char*)&inbuf->data[inbuf->len],
		results1, regexExpr);

	printf("\nstd::regex_search\n");
	printf("    return value  : %s\n", r ? "true" : "false");
	printf("    search results: (%u)\n", results1.size());
	for (unsigned int idx = 0; idx < results1.size(); ++idx)
	{
		printf("        %u)\n", idx);
		printf("            %-7.7s: %u\n", "pos", results1.position(idx));
		printf("            %-7.7s: %u\n", "length", results1.length(idx));
		printf("            %-7.7s: %s\n", "str", results1.str(idx).c_str());
	}

#pragma endregion
	return 0;
}
