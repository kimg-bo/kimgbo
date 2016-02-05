#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "FileUtil.h"

using namespace kimgbo;

FileUtil::SmallFile::SmallFile(StringPiece filename)
  : m_fd(::open(filename.data(), O_RDONLY | O_CLOEXEC)),
    m_err(0)
{
  m_buf[0] = '\0';
  if (m_fd < 0)
  {
    m_err = errno;
  }
}

FileUtil::SmallFile::~SmallFile()
{
  if (m_fd >= 0)
  {
    ::close(m_fd); // FIXME: check EINTR
  }
}

// return errno
template<typename String>
int FileUtil::SmallFile::readToString(int maxSize,
                                      String* content,
                                      int64_t* fileSize,
                                      int64_t* modifyTime,
                                      int64_t* createTime)
{
  static_assert(sizeof(off_t) == 8, "sizeof(off_t) != 8");
  assert(content != NULL);
  int err = m_err;
  if (m_fd >= 0)
  {
    content->clear();

    if (fileSize)
    {
      struct stat statbuf;
      if (::fstat(m_fd, &statbuf) == 0)
      {
        if (S_ISREG(statbuf.st_mode))
        {
          *fileSize = statbuf.st_size;
          content->reserve(static_cast<int>(std::min(implicit_cast<int64_t>(maxSize), *fileSize)));
        }
        else if (S_ISDIR(statbuf.st_mode))
        {
          err = EISDIR;
        }
        if (modifyTime)
        {
          *modifyTime = statbuf.st_mtime;
        }
        if (createTime)
        {
          *createTime = statbuf.st_ctime;
        }
      }
      else
      {
        err = errno;
      }
    }

    while (content->size() < implicit_cast<size_t>(maxSize))
    {
      size_t toRead = std::min(implicit_cast<size_t>(maxSize) - content->size(), sizeof(m_buf));
      ssize_t n = ::read(m_fd, m_buf, toRead);
      if (n > 0)
      {
        content->append(m_buf, n);
      }
      else
      {
        if (n < 0)
        {
          err = errno;
        }
        break;
      }
    }
  }
  return err;
}

int FileUtil::SmallFile::readToBuffer(int* size)
{
  int err = m_err;
  if (m_fd >= 0)
  {
    ssize_t n = ::pread(m_fd, m_buf, sizeof(m_buf)-1, 0);
    if (n >= 0)
    {
      if (size)
      {
        *size = static_cast<int>(n);
      }
      m_buf[n] = '\0';
    }
    else
    {
      err = errno;
    }
  }
  return err;
}

template int FileUtil::readFile(StringPiece filename,
                                int maxSize,
                                string* content,
                                int64_t*, int64_t*, int64_t*);

template int FileUtil::SmallFile::readToString(
    int maxSize,
    string* content,
    int64_t*, int64_t*, int64_t*);

#ifndef MUDUO_STD_STRING
template int FileUtil::readFile(StringPiece filename,
                                int maxSize,
                                std::string* content,
                                int64_t*, int64_t*, int64_t*);

template int FileUtil::SmallFile::readToString(
    int maxSize,
    std::string* content,
    int64_t*, int64_t*, int64_t*);
#endif

