#include <iostream>

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <sys/types.h>
#include <dirent.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

#ifdef __linux__
//linux code goes here
void read_directory(const std::string& name, std::vector<std::string>& v)
{
    DIR* dirp = opendir(name.c_str());
    struct dirent * dp;
    while ((dp = readdir(dirp)) != NULL) {
        if((dp->d_name).find("avi") != std::string::npos)
            v.push_back(dp->d_name);
    }
    closedir(dirp);
}
#elif (_WIN32|__WIN32__|__WIN64)
// windows code goes here
#include <windows.h>
void read_directory(const std::string& name, std::vector<std::string>& v)
{
    std::string pattern(name);
    pattern.append("\\*avi");
    WIN32_FIND_DATA data;
    HANDLE hFind;
    if ((hFind = FindFirstFile(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE) {
        do {
            v.push_back(data.cFileName);
        } while (FindNextFile(hFind, &data) != 0);
        FindClose(hFind);
    }
}
#else
void read_directory(const std::string& name, std::vector<std::string>& v)
{
    std::cout<<"Not implemented" <<std::endl;
}
#endif
using namespace std;
using namespace cv;
typedef enum {
    VIDEO_FILE=0,
    AUDIO_FILE,
    IMAGE_FILE,
    INVALID_FILE
} file_type;

std::string file_path = "./";
std::string allowedChars = "abcdefghijklmnaoqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
std::string allowed_user_names[2] = {"camuser", "admin"};


//namespace fs = std::filesystem;

void listRecordedVids()
{
    std::vector<std::string> file_list_vec;
    read_directory(".", file_list_vec);
    std::vector<std::string>::const_iterator itr = file_list_vec.begin();
    std::cout<<"**************************"<<std::endl;
    std::cout<<"Found a total of "<<file_list_vec.size()<<" files"<<std::endl;
    std::cout<<"**************************"<<std::endl;
    unsigned int count_val = 0;
    for(;itr != file_list_vec.end();++itr)
        std::cout<<++count_val<<":"<<*itr<<std::endl;
    std::cout<<"**************************"<<std::endl;
//    std::copy(file_list_vec.begin(), file_list_vec.end(),
//         std::ostream_iterator<std::string>(std::cout, "\n"));
}


std::string generateRandomString(uint maxOutputLength = 15)
{
    uint outputLength = rand() % maxOutputLength + 1;

    int randomIndex;
    std::string outputString = "";

    for (uint i = 0; i < outputLength; ++i) {
        randomIndex = rand() % allowedChars.length();
        outputString += allowedChars[randomIndex];
    }

    if (outputString.empty()) {
        return generateRandomString(maxOutputLength);
    }
    else {
        return outputString;
    }
}



void generate_file_name(file_type curr_file_type, std::string& output_filename, std::string opt_prefix="")
{
    static unsigned int file_count = 1;
    std::string file_prefix = "";
    std::string file_suffix = "";
    switch(curr_file_type)
    {
        case VIDEO_FILE:
            file_prefix="vid_";
            file_suffix=".avi";
            break;
        case IMAGE_FILE:
            file_prefix="pic_";
            file_suffix=".png";
            break;
        default:
            file_prefix="data_";
            file_suffix=".dat";
    }

    std::string curr_file_name = generateRandomString(6);
    output_filename = file_path+file_prefix+curr_file_name+opt_prefix+file_suffix;
    std::cout<<"Filename:"<<output_filename<<std::endl;
}

void handleImgTransforms(std::vector<Mat>& frames_vector, VideoCapture& cap)
{
    unsigned int count_val =0;
    std::vector<Mat>::const_iterator itr = frames_vector.begin();
    std::cout<<"Captured frame count:"<<frames_vector.size()<<std::endl;
    int frame_width = cap.get(3);
    int frame_height = cap.get(4);

    // Define the codec and create VideoWriter object.The output is stored in 'outcpp.avi' file.
    std::string windowName = "Webcam Feed";
    namedWindow(windowName, WINDOW_AUTOSIZE);
    std::string vid_file_name = "";
    generate_file_name(VIDEO_FILE, vid_file_name);
    int fourcc = VideoWriter::fourcc('M', 'J', 'P', 'G');
    VideoWriter video(vid_file_name,fourcc,5, Size(frame_width,frame_height));
    for(;itr != frames_vector.end();++itr)
    {
        ++count_val;
        vector<Mat> bgr; // For splitting and extracting the channels
        Mat res;  // For Final combination
        cv::split(*itr,bgr); // Split the frame into B,G,R channels
        // Zero out rest of the channels
        bgr[0] = Mat::zeros(Size(frame_width,frame_height), bgr[0].type());
        bgr[1] = Mat::zeros(Size(frame_width,frame_height), bgr[0].type());

        Mat gblur_op = bgr[2].clone();
        // Create Gaussian blur only for the red channel.
        for ( int i = 1; i < 20; i = i + 2 )
        {
            GaussianBlur( bgr[2], gblur_op, Size( i, i ), 0, 0 );
        }
        bgr[2]=gblur_op;

        cv::merge(bgr, res);
        imshow(windowName, res);
        video.write(res);
    }
}

void recordVid(unsigned int time_dur =10)
{

	time_t curr_time = time(NULL);
	time_t end_time = curr_time+time_dur;
	//std::cout<< "Curr time "<<curr_time<<" end time "<<end_time<<" diff "<<difftime(end_time, curr_time)<<std::endl;
	//unsigned int frame_width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    //unsigned int frame_height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
    std::vector<Mat> frames_vec;
    VideoCapture cap(0);
    int ex = static_cast<int>(cap.get(CAP_PROP_FOURCC));
    //char EXT[] = {(char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0};

    //std::cout <<"fourcc="<<EXT<<" fps="<<cap.get(CAP_PROP_FPS)<<std::endl;
    cap.set(CAP_PROP_FPS, 5);
    //unsigned int count_val=0;
	while(difftime(end_time, curr_time) > 0 )
    {
        std::cout<<"Starting capture"<<std::endl;

        if (!cap.isOpened()) {
            std::cout << "Error initializing video camera!" << endl;
            return;
        }
        Mat frame;
        cap >> frame;
        if(frame.empty())
        {
            std::cout<<"Empty frame"<<std::endl;
            curr_time=time(NULL);
            continue;
        }

		frames_vec.insert(frames_vec.end(), frame);
        std::cout<<"Captured one frame"<<std::endl;
		curr_time=time(NULL);

    }
    std::cout<<"Displaying the captured frames"<<std::endl;
    handleImgTransforms(frames_vec,cap);
    cap.release();
    destroyAllWindows();
}

bool login_user(std::string& set_user_name)
{

    //std::vector<std::string> allowed_user_names;
    std::string user_name;
    std::cout<<"Please provide your user name" <<std::endl;
    std::cin>> user_name;
    std::cout<<"Hello " << user_name<<std::endl;
    for(unsigned int iter=0; iter < 2;++iter)
    {
        //std::cout << allowed_user_names[iter]<<std::endl;
        if(allowed_user_names[iter] == (user_name))
        {
            set_user_name=user_name;
            return true;
        }

    }
    std::cout<<"Sorry that was an invalid entry"<<std::endl;
    return false;
}

void display_menu_on_login_success()
{
    bool logout = false;
    while(logout == false)
    {
        unsigned char select_opt = 'a';
        std::cout<<"Please select one of the option from below"<<std::endl;
        std::cout<<"Press 'c' for recording new video"<<std::endl;
        std::cout<<"Press 's' for listing the recorded videos" <<std::endl;
        std::cout<<"Press 'l' to logout"<<std::endl;

        std::cin>>select_opt;
        switch(select_opt)
        {
            case 'c':
            case 'C':
                recordVid();
                break;
            case 's':
            case 'S':
                listRecordedVids();
                break;
            case 'b':
            default:
                logout = true;
                break;
        }
    }
}

int main() {
    std::string login_user_name="";
	while (1)
    {
        if (login_user(login_user_name) != true)
        {
            std::cout<<"Invalid username!" <<std::endl;
            continue;
        }
        std::cout<<"Welcome " <<login_user_name<<std::endl;
        //std::cout<<"Please look at the webcam for 10secs"<<std::endl;
        //recordVid();
        display_menu_on_login_success();
	}
	return 0;
}
