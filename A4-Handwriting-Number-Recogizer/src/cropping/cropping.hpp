#ifndef CROPPING_HPP
#define CROPPING_HPP
#include<CImg.h>
#include<vector>
#include<queue>
#include<algorithm>
#include<cmath>
#include<map>
using namespace std;
using namespace cimg_library;
typedef vector<int> BBOX;

void crop_rectangel(CImg<unsigned char> &img) {
    // left, right, top, down
    int bbox[4] = {0, 0, 0, 0};
    bbox[0] = img.width()-1;
    bbox[2] = img.height()-1;

    int calculator = 0;
    for (int h = 20; h < img.height()-20; h++) {
        // find left
        calculator = 0;
        for (int w = 0; w <= bbox[0]; w++) {
            if (img(w, h) > 0) calculator++;
            else if (img(w, h) == 0 && calculator > 30) {
                if (bbox[0] > w) bbox[0] = w;
                break;
            } 
        }
        // find right
        calculator = 0;
        for (int w = img.width()-1; w >= bbox[1]; w--) {
            if (img(w, h) > 0) calculator++;
            else if (img(w, h) == 0 && calculator > 30) {
                if (bbox[1] < w) bbox[1] = w;
                break;
            } 
        }
    }

    for (int w = 20; w < img.width()-20; w++) {
        // find top
        calculator = 0;
        for (int h = 0; h <= bbox[2]; h++) {
            if (img(w, h) > 0) calculator++;
            else if (img(w, h) == 0 && calculator > 30) {
                if (bbox[2] > h) bbox[2] = h;
                break;
            } 
        }
        // find down
        calculator = 0;
        for (int h = img.height()-1; h >= bbox[3]; h--) {
            if (img(w, h) > 0) calculator++;
            else if (img(w, h) == 0 && calculator > 30) {
                if (bbox[3] < h) bbox[3] = h;
                break;
            } 
        }
    }

    // crop rectangel
    img = img.crop(bbox[0], bbox[2], 0, 0, bbox[1], bbox[3], 0, 0);
    img = 255-img;
    // img.display();
}

int w_bias[] = {-1, -1, -1, 0, 0, 1, 1, 1, 0, 0};
int h_bias[] = {-1, 0, 1, -1, 1, -1, 0, 1, -2, 2, -2, 2, -2, 2};
BBOX fill_connected(CImg<unsigned char> &img, CImg<long> &mark, int width, int height, vector<long> &label_count) {
    priority_queue<pair<int, int> > node_queue;

    // left right top down
    BBOX number_bbox(4);
    number_bbox[0] = number_bbox[1] = width;
    number_bbox[2] = number_bbox[3] = height;
    node_queue.push(pair<int, int>(width, height));
    int label = label_count.size();
    while (!node_queue.empty()) {
        pair<int, int> top = node_queue.top();
        node_queue.pop();
        for (int b = 0; b < 8; b++) {
            int w = top.first + w_bias[b];
            int h = top.second + h_bias[b];

            if (w >= 0 && h >= 0 && w < img.width() && h < img.height()) {
                if (img(w, h) > 0 && mark(w, h) == 0) {
                    node_queue.push(pair<int, int>(w, h));
                    label_count[label-1]++;
                    mark(w, h) = label;

                    // update bbox
                    if (number_bbox[0] > w) number_bbox[0] = w;
                    if (number_bbox[1] < w) number_bbox[1] = w;
                    if (number_bbox[2] > h) number_bbox[2] = h;
                    if (number_bbox[3] < h) number_bbox[3] = h;
                }
            }
        }
    }
    return number_bbox;
}

// BBOX 全局比较函数，排序左右位置
bool less_left(const BBOX& bbox1, const BBOX& bbox2){
    return bbox1[0] < bbox2[0];
}
// BBO 全局比较函数，排序上下位置
bool less_top(const BBOX& bbox1, const BBOX& bbox2){
    double c1 = (bbox1[2] + bbox1[3]) / 2;
    double c2 = (bbox2[2] + bbox2[3]) / 2;
    double margin = (bbox1[3] - bbox1[2] + bbox2[3] - bbox2[2]) / 3;
    if (abs(c1 - c2) < margin)
        return bbox1[0] < bbox2[0];
    else return c1 < c2;
}
void cluster_lines(vector<BBOX> number_bboxes, vector<vector<BBOX> > &lines) {
    sort(number_bboxes.begin(), number_bboxes.end(), less_top);

    // collect bbox in same line approximately
    for (int i = 0; i < number_bboxes.size(); i++) {
        vector<BBOX> line;
        line.push_back(number_bboxes[i]);
        int j;
        int top_h = number_bboxes[i][2];
        int down_h = number_bboxes[i][3];
        for (j = i+1; j < number_bboxes.size(); j++) {
            if (number_bboxes[j][2] < down_h && number_bboxes[j][3] > top_h) {
                double union_len = min(number_bboxes[j][3], down_h)-max(number_bboxes[j][2], top_h);
                double u_r = union_len / (number_bboxes[j][3] - number_bboxes[j][2]);
                if (u_r > 0.2) {
                    line.push_back(number_bboxes[j]);
                    if (number_bboxes[j][2] < top_h) top_h = number_bboxes[j][2];
                    if (number_bboxes[j][3] > down_h) down_h = number_bboxes[j][3];
                }
                else break;
            }
            else break;
            // if (number_bboxes[j][2] < number_bboxes[j-1][3] && number_bboxes[j][3] > number_bboxes[j-1][2]) {
            //     line.push_back(number_bboxes[j]);
            // }
            // else break;
        }
        i = j-1;

        // sort in line
        if (line.size() > 4) {
            sort(line.begin(), line.end(), less_left);
            lines.push_back(line);
        }
        if (lines.size() >= 9) return;
    }

    // remove the bbox out of range
    // for (int i = 0; i < lines.size(); i++) {

    // }
}

void draw_bbox(CImg<unsigned char> &img, vector<BBOX> &bboxes) {
    for (int i = 0; i < bboxes.size(); i++) {
        for (int w = bboxes[i][0]-5; w <= bboxes[i][1]+5; w++) {
            if (w >= 0 && w < img.width()) {
                if (bboxes[i][2]-5 >= 0 && bboxes[i][2]-5 < img.height()) img(w, bboxes[i][2]-5) = 255;
                if (bboxes[i][3]+5 >= 0 && bboxes[i][3]+5 < img.height()) img(w, bboxes[i][3]+5) = 255;
            }
        }
        for (int h = bboxes[i][2]-5; h <= bboxes[i][3]+5; h++) {
            if (h >= 0 && h < img.height()) {
                if (bboxes[i][0]-5 >= 0 && bboxes[i][0]-5 < img.width()) img(bboxes[i][0]-5, h) = 255;
                if (bboxes[i][1]+5 >= 0 && bboxes[i][1]+5 < img.width()) img(bboxes[i][1]+5, h) = 255;
            }
        }
    }
}

// 获取一个bbox里的非0众数
int get_label(CImg<long> &mark, BBOX &b) {
    map<int, int> m;
    for (int h = b[2]; h <= b[3]; h++) {
        for (int w = b[0]; w <= b[1]; w++) {
            if (mark(w, h) != 0) {
                map<int ,int>::iterator iter = m.find(mark(w, h));
                if (iter == m.end()) {
                    m.insert(pair<int, int>(mark(w, h), 1));
                }
                else {
                    iter->second++;
                }
            }
        }
    }

    // find max
    int m_l = 0;
    int m_n = -1;
    for (map<int, int>::iterator i = m.begin(); i != m.end(); i++) {
        if (i->second > m_n) {
            m_n = i->second;
            m_l = i->first;
        }
    }
    return m_l;
}

// 通过6x6的矩阵内是否存在不同标号，判断是否连接，或者中位线距离足够近
bool is_merge(CImg<long> &mark, BBOX &bu, BBOX &b1, BBOX &b2) {
    double margin_c = abs((b1[0]+b1[1]) - (b2[0]+b2[1]))/2;
    if (margin_c < 10) return true;

    int label1 = get_label(mark, b1);
    int label2 = get_label(mark, b2);
    // printf("%d %d\n", label1, label2);
    for (int h = bu[2]; h <= bu[3]-5 && h < mark.height()-5; h++) {
        for (int w = bu[0]; w <= bu[1]-5 && w < mark.width()-5; w++) {
            bool l1_ = false, l2_ = false;
            for (int h_b = 0; h_b < 6; h_b++) {
                for (int w_b = 0; w_b < 6; w_b++) {
                    int label_i = mark(w + w_b, h + h_b);
                    if (label_i == label1) l1_ = true;
                    else if (label_i == label2) l2_ = true;
                    if (l1_ && l2_) return true;
                }
            }
        }
    }
    return false;
}
void crop_number(CImg<unsigned char> &img, CImg<long> &mark, vector<vector<BBOX> > &lines) {
    // input img has only one channel
    crop_rectangel(img);

    // 积分图
    // CImg<float> iimg = CImg<float>(img);
    // for (int h = 0; h < iimg.height(); h++) {
    //     for (int w = 0; w < iimg.width(); w++) {
    //         float ul=0, ur=0, ll=0, lr=iimg(w, h);
    //         if (w != 0) ll = iimg(w-1, h);
    //         if (h != 0 && w != 0) ul = iimg(w-1, h-1);
    //         if (h != 0) ur = iimg(w, h-1);
    //         iimg(w, h) = lr + (ll - ul) + ur;
    //     }
    // }
    // iimg.display();

    // 连通区域分割
    vector<long> label_count;
    mark = CImg<long>(img.width(), img.height(), 1, 1, 0);
    vector<BBOX> number_bboxes;
    for (int h = 0; h < img.height(); h++) {
        for (int w = 0; w < img.width(); w++) {
            if (img(w, h) > 0 && mark(w, h) == 0) {
                label_count.push_back(1);
                mark(w, h) = label_count.size();
                BBOX bbox = fill_connected(img, mark, w, h, label_count);
                // 过滤取bbox
                if (label_count[mark(w, h)-1] > 50 && label_count[mark(w, h)-1] < 2800) {
                    // img.get_crop(bbox[0]-5, bbox[2]-5, 0, 0, bbox[1]+5, bbox[3]+5, 0, 0).display();
                    number_bboxes.push_back(bbox);
                }
                // else {
                //     img.get_crop(bbox[0]-5, bbox[2]-5, 0, 0, bbox[1]+5, bbox[3]+5, 0, 0).display();
                //     printf("%d, %d %d\n", label_count[mark(w, h)-1], w, h);
                // }
            }
        }
    }
    // mark.display();
    
    // 找出各行
    cluster_lines(number_bboxes, lines);


    // process bbox
    for (int i = 0; i < lines.size(); i++) {
        for (int b = 0; b < lines[i].size(); b++) {
            // merge
            if (b != lines[i].size()-1 && lines[i][b][0]-5 <= lines[i][b+1][1]+5 && lines[i][b][1]+5 >= lines[i][b+1][0]-5
                && lines[i][b][2]-5 <= lines[i][b+1][3]+5 && lines[i][b][3]+5 >= lines[i][b+1][2]-5) {
                // 如果相交.
                BBOX bu;
                bu.push_back(min(lines[i][b][0], lines[i][b+1][0]));
                bu.push_back(max(lines[i][b][1], lines[i][b+1][1]));
                bu.push_back(min(lines[i][b][2], lines[i][b+1][2]));
                bu.push_back(max(lines[i][b][3], lines[i][b+1][3]));
                float area1 = (lines[i][b][1] - lines[i][b][0])*(lines[i][b][3] - lines[i][b][2]);
                float area2 = (lines[i][b+1][1] - lines[i][b+1][0])*(lines[i][b+1][3] - lines[i][b+1][2]);
                float area3 = (bu[1] - bu[0]) * (bu[3] - bu[2]);

                float r1 = area1 / area3;
                float r2 = area2 / area3;
                if (r1 < 0.2 || r2 < 0.2) {
                    // 判断两个目标是不是很近

                    if (is_merge(mark, bu, lines[i][b], lines[i][b+1])) {
                        // merge
                        lines[i][b][0] = bu[0];
                        lines[i][b][1] = bu[1];
                        lines[i][b][2] = bu[2];
                        lines[i][b][3] = bu[3];

                        lines[i].erase(lines[i].begin() + (b+1));
                        b--;
                    }
                }

                // printf("%d, %d %d\n", i, b, b+1);
                // if (is_merge(mark, bu, lines[i][b], lines[i][b+1])) {
                //     // merge
                //     lines[i][b][0] = bu[0];
                //     lines[i][b][1] = bu[1];
                //     lines[i][b][2] = bu[2];
                //     lines[i][b][3] = bu[3];

                //     lines[i].erase(lines[i].begin() + (b+1));
                // }
            }
        }
    }

    // split by number
    // 学号8位、号码11位、身份证号18位
    for (int i = 0; i < lines.size(); i++) {
        if ((i % 3 == 0 && lines[i].size() != 8) || (i % 3 == 1 && lines[i].size() != 11) || (i % 3 == 2 && lines[i].size() != 18) )
            for (int b = 0; b < lines[i].size(); b++) {
                float r = (float)(lines[i][b][1] - lines[i][b][0])/ (lines[i][b][3] - lines[i][b][2]);
                if (r >= 1.2) {
                    BBOX new_bbox;
                    new_bbox.push_back((int)((lines[i][b][1] + lines[i][b][0]) / 2));
                    new_bbox.push_back(lines[i][b][1]);
                    new_bbox.push_back(lines[i][b][2]);
                    new_bbox.push_back(lines[i][b][3]);
                    lines[i][b][1] = new_bbox[0];

                    lines[i].insert(lines[i].begin()+(b+1), new_bbox);
                    break;
                }
            }
    }

    
    // draw bbox
    for (int i = 0; i < lines.size(); i++) {
        // for (int j = 0; j < lines[i].size(); j++) {
        //     string file_path = "output/bbox/" + to_string(i) + "-" + to_string(j) + ".bmp";
        //     img.get_crop(lines[i][j][0], lines[i][j][2], 0, 0, lines[i][j][1], lines[i][j][3], 0, 0).save(file_path.c_str());
        // }
        draw_bbox(img, lines[i]);
    }
    // img.display();
    // img.save("output/bbox.jpg");
    
    
}

#endif