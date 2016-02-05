#ifndef FILEUTIL_H
#define FILEUTIL_H

#include "Types.h"
#include "StringPiece.h"

namespace kimgbo
{

namespace FileUtil
{

  class SmallFile
  {
   public:
    SmallFile(StringPiece filename);
    ~SmallFile();

    // return errno
    template<typename String>
    int readToString(int maxSize,
                     String* content,
                     int64_t* fileSize,
                     int64_t* modifyTime,
                     int64_t* createTime);

    // return errno
    int readToBuffer(int* size);

    const char* buffer() const { return m_buf; }

    static const int kBufferSize = 65536;

   private:
    int m_fd;
    int m_err;
    char m_buf[kBufferSize];
  };

  // read the file content, returns errno if error happens.
  template<typename String>
  int readFile(StringPiece filename,
               int maxSize,
               String* content,
               int64_t* fileSize = NULL,
               int64_t* modifyTime = NULL,
               int64_t* createTime = NULL)
  {
    SmallFile file(filename);
    return file.readToString(maxSize, content, fileSize, modifyTime, createTime);
  }

}

}

#endif  // MUDUO_BASE_FILEUTIL_H

