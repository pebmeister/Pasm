#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

void Usage()
{
    cout << "Usage:" << endl 
        << "\tMergeRelocate relocate.prg mon1.prg mon2.prg outputfile.prg" << endl;
}

vector<unsigned char> ReadFile(char* name)
{
    ifstream input(name, ios::binary);
    if (!input.is_open())
    {
        cout << "Unable to open " << name << endl;
        Usage();
        exit(1);
    }

    // copies all data into buffer
    vector<unsigned char> buffer(istreambuf_iterator<char>(input), {});
    input.close();
    return buffer;
}

int GetInt(vector<unsigned char>& buffer, const int offset)
{
    // Little Endian
    return static_cast<int>(static_cast<unsigned int>(buffer[offset]) + 
        static_cast<unsigned int>(buffer[offset + 1]) * 256);
}

int main(const int argc, char* argv[])
{
    if (argc != 5)
    {
        Usage();
        return 0;
    }

    const unsigned char marker = 0x36;

    vector<unsigned char> stub = ReadFile(argv[1]);
    vector<unsigned char> mon1 = ReadFile(argv[2]);
    vector<unsigned char> mon2 = ReadFile(argv[3]);

    if (mon1.size() != mon2.size())
    {
        cout << "input files are not the same length" << endl;
        exit(1);
    }

    ofstream out(argv[4], ios::binary);
    if (!out.is_open())
    {
        cout << "Unable to open " << argv[4] << endl;
        Usage();
        exit(1);
    }

    // # get end of first program from header
    const unsigned long long end1 = GetInt(mon1,0) + static_cast<int>(mon1.size()) - 2;

    // # strip origin from both programs
    mon1.erase(mon1.begin(), mon1.begin() + 2);
    mon2.erase(mon2.begin(), mon2.begin() + 2);

    // # prepend relocator stub
    for (char stub1 : stub)
        out << stub1;

    // # separate from supermon code with $36 twice
    out << marker << marker;

    unsigned long i = 0u;
    const auto mon1Sz = static_cast<unsigned long>(mon1.size());
    while (i < mon1Sz)
    {
        if (mon1Sz > i + 1 && mon1[i + 1] != mon2[i + 1])
        {
            const int addr = GetInt(mon1, static_cast<int>(i));

            // # calculate offset to address
            const int offset = addr - static_cast<int>(end1);

            // write in Little Endian
            out << static_cast<unsigned char>(offset & 0xFF);
            out << static_cast<unsigned char>(offset >> 8);

            // # mark address to be adjusted with $36
            out << marker;
            i += 2;
        }
        else
        {
            out << mon1[i];
            i += 1;
        }
    }

    out.close();
}
