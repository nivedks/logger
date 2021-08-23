# logger

# Steps to build
1. Install cmake
2. In the root folder run "cmake ."
3. make

# Steps to run
./src/Logger

# Configuration options
The below macros are defined in ./inc/Logger.h

To toggle between stdout and file logging use the below macro, the presence of the
macro enables file logging
ENABLE_FILE_LOGGING

Number of files to be used during file logging
MAX_NUMBER_FILES 5

Max size of each file to be used during file logging
MAX_FILE_SIZE_IN_BYTES 2000

Internal buffer size of the logger library
LOGGER_INTERNAL_BUFFER_SIZE 100