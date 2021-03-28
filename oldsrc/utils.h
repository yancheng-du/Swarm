#ifndef TF_DETECTOR_EXAMPLE_UTILS_H
#define TF_DETECTOR_EXAMPLE_UTILS_H

#endif //TF_DETECTOR_EXAMPLE_UTILS_H

#include <vector>
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"
#include <opencv2/core/mat.hpp>


using tensorflow::Tensor;
using tensorflow::Status;
using tensorflow::string;

Status ReadLabelsFile(const string& file_name, std::vector<string>* result,
                      size_t* found_label_count);

Status readLabelsMapFile(const string &fileName, std::map<int, string> &labelsMap);

static Status ReadEntireFile(tensorflow::Env* env, const string& filename,
                             Tensor* output);

Status GetTopLabels(const std::vector<Tensor>& outputs, int how_many_labels,
                    Tensor* indices, Tensor* scores);

Status CheckTopLabel(const std::vector<Tensor>& outputs, int expected,
                     bool* is_expected);

Status PrintTopLabels(const std::vector<Tensor>& outputs,
                      const string& labels_file_name);

Status ReadTensorFromImageFile(const string& file_name, const int input_height,
                               const int input_width, const float input_mean,
                               const float input_std,
                               std::vector<Tensor>* out_tensors);
Status loadGraph(const string &graph_file_name,
                 std::unique_ptr<tensorflow::Session> *session);

Status readTensorFromMat(const cv::Mat &mat, Tensor &outTensor);

void detect(cv::Rect &rec, std::unique_ptr<tensorflow::Session> &session2, cv::Mat &image, double xMin, double yMin, double xMax, double yMax, double score, bool* is_expected, bool scaled);

void detect(cv::Rect &rec, std::unique_ptr<tensorflow::Session> &session2, cv::Mat &image,
                              tensorflow::TTypes<float>::Flat &scores,
                              tensorflow::TTypes<float,3>::Tensor &boxes,
                              std::vector<size_t> &idxs, bool* is_expected);

double IOU(cv::Rect box1, cv::Rect box2);

std::vector<size_t> filterBoxes(tensorflow::TTypes<float>::Flat &scores,
                                tensorflow::TTypes<float, 3>::Tensor &boxes,
                                double thresholdIOU, double thresholdScore);
