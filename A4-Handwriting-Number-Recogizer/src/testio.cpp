#include <iostream>
#include <string>
#include <vector>
#include <algorithm>  // transform().
#include <cctype>  // tolower().
using namespace std;

#include <boost/filesystem.hpp>
using namespace boost::filesystem;



int getImageList(int &argc, path p, vector<string> &imgPathList) {
    if (argc < 2) {
        cout << "Usage: tut3 path\n";
        return 1;
    }

    try {
        if (exists(p)) {
            if (is_regular_file(p))
                cout << p << " size is " << file_size(p) << '\n';
            else if (is_directory(p)) {
                cout << p << " is a directory containing:\n";
                for (directory_entry& x : directory_iterator(p)) {
                    if (is_regular_file(x.path())) {
                        string temp_path = x.path().string();
                        string temp_lower = temp_path;
                        transform(temp_path.begin(), temp_path.end(), temp_lower.begin(), ::tolower);  // Why?  static_cast<int(*)(int)>std::tolower
                        
                        int str_size = temp_lower.size();
                        int png_pos = temp_lower.rfind(".png");
                        int jpg_pos = temp_lower.rfind(".jpg");
                        int jpeg_pos = temp_lower.rfind(".jpeg");
                        int bmp_pos = temp_lower.rfind(".bmp");
                        if (png_pos+4==str_size || jpg_pos+4==str_size || jpeg_pos+5==str_size || bmp_pos+4==str_size)
                            imgPathList.push_back(temp_path);
                    }
                }
            }
            else
                cout << p << " exists, but is not a regular file or directory\n";
        }
        else
        cout << p << " does not exist\n";
    }

    catch (const filesystem_error& ex) {
        cout << ex.what() << '\n';
    }

    return 0;
}


int main(int argc, char* argv[]) {
    vector<string> imgPathList;
    int res = getImageList(argc, path(argv[1]), imgPathList);
    if (res == 1) return 1;

    for (int i = 0; i < imgPathList.size(); i++) {
        cout << imgPathList[i] << endl;
    }
    return 0;
}