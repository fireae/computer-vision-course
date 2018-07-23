#include <iostream>
#ifndef IO_INTERFACE_HPP
#define IO_INTERFACE_HPP

#include <string>
#include <vector>
#include <algorithm>  // transform().
#include <cctype>  // tolower().
#include <fstream>
using namespace std;

#include <boost/filesystem.hpp>
using namespace boost::filesystem;



int getImageList(path p, vector<string> &imgPathList) {
    try {
        if (exists(p)) {
            if (is_regular_file(p)) {
                cout << p << " is a file\n";
                string ext = p.extension().string();
                if (ext==".png" || ext==".jpg" || ext==".jpeg" || ext==".bmp")
                    imgPathList.push_back(p.string());
            }
            else if (is_directory(p)) {
                cout << p << " is a directory containing:\n";
                for (directory_entry& x : directory_iterator(p)) {
                    if (is_regular_file(x.path())) {
                        string temp_path = x.path().string();
                        string temp_lower = temp_path;
                        transform(temp_path.begin(), temp_path.end(), temp_lower.begin(), ::tolower);  // Why?  static_cast<int(*)(int)>std::tolower
                        

                        // Can also use Path.extension()
                        string ext = path(temp_lower).extension().string();
                        if (ext==".png" || ext==".jpg" || ext==".jpeg" || ext==".bmp")
                            imgPathList.push_back(temp_path);
                    }
                }
                cout << "Find " << imgPathList.size()+1 << " images in " << p << ".\n";
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


void saveCSVResult(string input_path, string output_dir, vector<vector<int>> &number_list) {
    if (output_dir.rfind("/") != output_dir.size()-1) {
        output_dir += "/";
    }
    string output_path = output_dir + path(input_path).filename().string();
    string extension = path(input_path).extension().string();

    output_path.replace(output_path.rfind(extension), extension.size(), ".csv", 0, 4);

    std::ofstream f;

    f.open(output_path.c_str());
    f << "number\n";
    for (int i = 0; i < number_list.size(); i++) {
        for (int j = 0; j < number_list[i].size(); j++) {
            f << number_list[i][j];
        }
        f << "\n";
    }
    f.close();

}

#endif
