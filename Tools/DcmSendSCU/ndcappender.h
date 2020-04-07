#ifndef __APPENDER_H__
#define __APPENDER_H__

#include <boost/filesystem.hpp>

// work around the fact that dcmtk doesn't work in unicode mode, so all string operation needs to be converted from/to mbcs
#ifdef _UNICODE
#undef _UNICODE
#undef UNICODE
#define _UNDEFINEDUNICODE
#endif

#include "dcmtk/config/osconfig.h"   /* make sure OS specific configuration is included first */
#include "dcmtk/oflog/appender.h"
#include "dcmtk/oflog/spi/logevent.h"

#ifdef _UNDEFINEDUNICODE
#define _UNICODE 1
#define UNICODE 1
#endif


class NDCAsFilenameAppender : public dcmtk::log4cplus::Appender
{
public:
	NDCAsFilenameAppender(const boost::filesystem::path& log_dir);
	virtual ~NDCAsFilenameAppender();

	virtual void close();

protected:
	virtual void append(const dcmtk::log4cplus::spi::InternalLoggingEvent& event);

	boost::filesystem::path path_;
};

#endif // __APPENDER_H__