/* 
 * Copyright (c) 2018 Flight Dynamics and Control Lab
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/core.hpp>
#include <vector>


void drawCubeWireframe(
    cv::InputOutputArray _image, cv::InputArray _cameraMatrix, 
    cv::InputArray _distCoeffs, cv::InputArray _rvec, cv::InputArray _tvec, 
    float l
);


int main(int argc, char **argv)
{
    int wait_time = 10;
    float actual_marker_l = 0.101; // this should be in meters

    cv::Mat image, image_copy;
    cv::Mat camera_matrix, dist_coeffs;
    std::ostringstream vector_to_marker;

    cv::VideoCapture in_video;
    in_video.open(0);
    cv::Ptr<cv::aruco::Dictionary> dictionary =
        cv::aruco::getPredefinedDictionary(cv::aruco::DICT_ARUCO_ORIGINAL);

    cv::FileStorage fs("../../calibration_params.yml", cv::FileStorage::READ);

    fs["camera_matrix"] >> camera_matrix;
    fs["distortion_coefficients"] >> dist_coeffs;

    std::cout << "camera_matrix\n"
              << camera_matrix << std::endl;
    std::cout << "\ndist coeffs\n"
              << dist_coeffs << std::endl;

    while (in_video.grab())
    {

        in_video.retrieve(image);
        image.copyTo(image_copy);
        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f>> corners;
        cv::aruco::detectMarkers(image, dictionary, corners, ids);

        // if at least one marker detected
        if (ids.size() > 0)
        {
            cv::aruco::drawDetectedMarkers(image_copy, corners, ids);
            std::vector<cv::Vec3d> rvecs, tvecs;
            cv::aruco::estimatePoseSingleMarkers(
                corners, actual_marker_l, camera_matrix, dist_coeffs, 
                rvecs, tvecs
            );

            // draw axis for each marker
            for (int i = 0; i < ids.size(); i++)
            {
                drawCubeWireframe(
                    image_copy, camera_matrix, dist_coeffs, rvecs[i], tvecs[i], 
                    actual_marker_l
                );

                vector_to_marker.str(std::string());
                vector_to_marker << std::setprecision(4)
                                 << "x: " << std::setw(8) << tvecs[0](0);
                cv::putText(image_copy, vector_to_marker.str(),
                            cvPoint(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.6,
                            cvScalar(0, 252, 124), 1, CV_AA);

                vector_to_marker.str(std::string());
                vector_to_marker << std::setprecision(4)
                                 << "y: " << std::setw(8) << tvecs[0](1);
                cv::putText(image_copy, vector_to_marker.str(),
                            cvPoint(10, 50), cv::FONT_HERSHEY_SIMPLEX, 0.6,
                            cvScalar(0, 252, 124), 1, CV_AA);

                vector_to_marker.str(std::string());
                vector_to_marker << std::setprecision(4)
                                 << "z: " << std::setw(8) << tvecs[0](2);
                cv::putText(image_copy, vector_to_marker.str(),
                            cvPoint(10, 70), cv::FONT_HERSHEY_SIMPLEX, 0.6,
                            cvScalar(0, 252, 124), 1, CV_AA);
            }
        }

        cv::imshow("Pose estimation", image_copy);
        char key = (char)cv::waitKey(wait_time);
        if (key == 27)
            break;
    }

    in_video.release();
}

void drawCubeWireframe(
    cv::InputOutputArray _image, cv::InputArray _cameraMatrix, 
    cv::InputArray _distCoeffs, cv::InputArray _rvec, cv::InputArray _tvec, 
    float l
)
{

    CV_Assert(
        _image.getMat().total() != 0 &&
        (_image.getMat().channels() == 1 || _image.getMat().channels() == 3)
    );
    CV_Assert(l > 0);
    float half_l = l / 2.0;

    // project cube points
    std::vector<cv::Point3f> axisPoints;
    axisPoints.push_back(cv::Point3f(half_l, half_l, l));
    axisPoints.push_back(cv::Point3f(half_l, -half_l, l));
    axisPoints.push_back(cv::Point3f(-half_l, -half_l, l));
    axisPoints.push_back(cv::Point3f(-half_l, half_l, l));
    axisPoints.push_back(cv::Point3f(half_l, half_l, 0));
    axisPoints.push_back(cv::Point3f(half_l, -half_l, 0));
    axisPoints.push_back(cv::Point3f(-half_l, -half_l, 0));
    axisPoints.push_back(cv::Point3f(-half_l, half_l, 0));

    std::vector<cv::Point2f> imagePoints;
    projectPoints(
        axisPoints, _rvec, _tvec, _cameraMatrix, _distCoeffs, imagePoints
    );

    // draw cube edges lines
    line(_image, imagePoints[0], imagePoints[1], cv::Scalar(255, 0, 0), 3);
    line(_image, imagePoints[0], imagePoints[3], cv::Scalar(255, 0, 0), 3);
    line(_image, imagePoints[0], imagePoints[4], cv::Scalar(255, 0, 0), 3);
    line(_image, imagePoints[1], imagePoints[2], cv::Scalar(255, 0, 0), 3);
    line(_image, imagePoints[1], imagePoints[5], cv::Scalar(255, 0, 0), 3);
    line(_image, imagePoints[2], imagePoints[3], cv::Scalar(255, 0, 0), 3);
    line(_image, imagePoints[2], imagePoints[6], cv::Scalar(255, 0, 0), 3);
    line(_image, imagePoints[3], imagePoints[7], cv::Scalar(255, 0, 0), 3);
    line(_image, imagePoints[4], imagePoints[5], cv::Scalar(255, 0, 0), 3);
    line(_image, imagePoints[4], imagePoints[7], cv::Scalar(255, 0, 0), 3);
    line(_image, imagePoints[5], imagePoints[6], cv::Scalar(255, 0, 0), 3);
    line(_image, imagePoints[6], imagePoints[7], cv::Scalar(255, 0, 0), 3);
}