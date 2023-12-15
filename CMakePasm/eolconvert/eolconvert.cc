#include <iostream>
#include <filesystem>
#include <set>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <limits.h>

#ifdef WIN32
#include <shlobj.h>
#include <direct.h>
#endif

//--------------------------------------------------------------------------//
using namespace std;
namespace fs = filesystem;

int process_directories(const set<fs::path>& directories, const fs::path& source_root, const fs::path& install_path);
int process_directory(const fs::path& directory, const fs::path& source_root, const fs::path& install_path);
int process_files(const set<fs::path>& files, const fs::path& source_root, const fs::path& install_path);
int process_file(const fs::path& path, const fs::path& source_root, const fs::path& install_path);

bool is_eol(uint8_t ch);
int replace_eol(const fs::path& path, const fs::path& out_path);
bool is_text_file(const fs::path& path);

fs::path base_path(const fs::path& path, const fs::path& source_root, const fs::path& install_path);

#ifdef WIN32
// ReSharper disable once CppInconsistentNaming
#define realpath(N,R) _fullpath((R),(N), 1024)
#endif

//--------------------------------------------------------------------------//
int main(const int argc, char** argv)
{

    if (argc > 1)
    {
        fs::path source_root_path;
        fs::path install_root_path;


        const fs::path source_path = realpath(argv[1], NULL);
        if (!is_directory(source_path))
        {
            cout << source_path << " is not a directory.";
            return -1;
        }
        cout << "source path " << source_path << '\n';

        if (argc > 2)
        {
            source_root_path = argv[2];

            cout << "source root path " << source_root_path << '\n';
        }

#ifdef WIN32
        CHAR my_documents[MAX_PATH];
        if (const HRESULT result = SHGetFolderPath(nullptr, CSIDL_PERSONAL, nullptr, SHGFP_TYPE_CURRENT, my_documents); result == ERROR_SUCCESS)
        {
            strcat(my_documents, "\\pasm\\");
            install_root_path = realpath(my_documents, NULL);
        }

#else
        if (argc > 3)
        {
            install_root_path = realpath(argv[3], NULL);
            if (!is_directory(install_root_path))
            {
                cout << install_root_path << " is not a directory.";
                return -1;
            }
        }
#endif

        return (process_directory(source_path, source_root_path, install_root_path) > 0) ? 0 : -1;
    }
    return -1;
}

//--------------------------------------------------------------------------//
int process_files(const set<fs::path>& files, const fs::path& source_root, const fs::path& install_path)
{
    auto result = 0;

    for (auto& entry : files)
        result += process_file(entry, source_root, install_path);

    return result;
}

//--------------------------------------------------------------------------//
int process_file(const fs::path& path, const fs::path& source_root, const fs::path& install_path)
{
    auto result = 0;
    if (exists(path))        
    {
        const auto out_path = base_path(path, source_root, install_path);
        replace_eol(path, out_path);
        result++;
    }

    return result;
}

//--------------------------------------------------------------------------//
int process_directories(const set<fs::path>& directories, const fs::path& source_root, const fs::path& install_path)
{
    auto result = 0;
    for (auto& entry : directories)
        result += process_directory(entry, source_root, install_path);

    return result;
}

//--------------------------------------------------------------------------//
int process_directory(const fs::path& directory, const fs::path& source_root, const fs::path& install_path)
{
    set<fs::path> directories;      // contains directories
    set<fs::path> files;            // contains files

    const auto dest_path = base_path(directory, source_root, install_path);
    if (! exists(dest_path))
    {
        create_directory(dest_path);
    }

    auto result = 0;

    // iterate current directory
    for (auto& entry : fs::directory_iterator(directory))
    {
        // check if entry is a directory
        if (is_directory(entry))
        {            
            // add to directories
            directories.insert(entry.path());
        }
        else
        {
            // add to files
            files.insert(entry.path());
        }
    }

    // process files
    result += process_files(files, source_root, install_path);
    result += process_directories(directories, source_root, install_path);

    return result;
}

//--------------------------------------------------------------------------//
int replace_eol(const fs::path& path, const fs::path& out_path)
{
    if (exists(path))
    {
        cout << out_path.string().c_str() << '\n';

        if (auto is_text = is_text_file(path); !is_text)
        {
            if (exists(out_path))
            {
                filesystem::remove(out_path);
            }
            copy_file(path, out_path);
            return 1;
        }

        ifstream file;
        file.open(path.string().c_str(), ios::binary);
        vector<uint8_t> in_buf((istreambuf_iterator(file)), (istreambuf_iterator<char>()));
        file.close();

        stringstream ss;
        size_t index = 0;
        auto sz = in_buf.size();
        uint8_t last_eol = 0;
        auto count = 0;
        while (index < sz)
        {

            if (auto ch = in_buf[index++]; !is_eol(ch))
            {
                ss << ch;
                count = 0;
            }
            else
            {
                if (ch != last_eol)
                {
                    count++;
                    last_eol = ch;
                    if (count == 1)
                        ss << '\n';
                    else if (count == 2)
                        count = 0;
                    continue;
                }
                ss << '\n';
                count = 0;
            }
        }

        ofstream outfile;
        outfile.open(out_path, ios::out | ios::trunc);
        if (exists(out_path))
        {
            auto str = ss.str();

            outfile.seekp(0);
            outfile.write(str.c_str(), static_cast<streamsize>(str.length()));
            outfile.flush();
            outfile.close();
        }
    }

    return 1;
}

//--------------------------------------------------------------------------//
inline bool is_eol(const uint8_t ch)
{
    return ch == '\r' || ch == '\n';
}

//--------------------------------------------------------------------------//
fs::path base_path(const fs::path& path, const fs::path& source_root, const fs::path& install_path)
{

    if (source_root.empty() || install_path.empty())
        return path;

    //cout << "filename       " << path.filename() << '\n';
    //cout << "extension      " << path.extension() << '\n';
    //cout << "parent_path    " << path.parent_path() << '\n';
    //cout << "relative_path  " << path.relative_path() << '\n';
    //cout << "root_directory " << path.root_directory() << '\n';
    //cout << "root_name      " << path.root_name() << '\n';
    //cout << "root_path      " << path.root_path() << '\n';

    string p;
    string r;

    if (is_directory(path))
    {
        p = path.string() + source_root.string()[source_root.string().length() -1];
        r = source_root.string();
    }
    else
    {
        p = path.relative_path().string();
        r = source_root.relative_path().string();
    }
    const auto pos = p.find(r);
    if (pos != std::string::npos)
    {
        auto out = install_path.string() + p.substr(pos);
        return out;
    }

    return path;
}

bool is_text_file(const fs::path& path)
{
    if (exists(path))
    {
        ifstream file;
        file.open(path.string().c_str(), ios::binary);
        const vector<uint8_t> in_buf((istreambuf_iterator(file)), (istreambuf_iterator<char>()));
        file.close();

        return (std::all_of(in_buf.cbegin(), in_buf.cend(), [](const uint8_t ch)
            { return isprint(ch) || is_eol(ch); }));
    }

    return false;
}