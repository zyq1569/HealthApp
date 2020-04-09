#include "ndcappender.h"
#include <fstream>
#include <iostream>

NDCAsFilenameAppender::NDCAsFilenameAppender(const boost::filesystem::path& log_dir)
{
    path_ = log_dir;
}

NDCAsFilenameAppender::~NDCAsFilenameAppender()
{

}

void NDCAsFilenameAppender::close()
{

}

void NDCAsFilenameAppender::append(const dcmtk::log4cplus::spi::InternalLoggingEvent& event)
{
    // file open and output 
    boost::filesystem::path filename = path_;
    if(event.getNDC().length() != 0)
        filename /= (event.getNDC() + ".txt").c_str();
    else
        filename /= "main.txt";
    std::ofstream myfile;
    myfile.open(filename.c_str(), std::ios::app | std::ios::out);
    layout->formatAndAppend(myfile, event);    
}