#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <pthread.h>

#include <errno.h>
#include <fcntl.h>


// 		DIR *dp;
//         struct dirent *dirp;
//         struct stat sb;
//         int dfd = open(DataPath, O_RDONLY);
//         if (dfd == -1) {
//             fprintf(stderr, "Failed to open directory %s for reading (%d: %s)\n",
//                     DataPath, errno, strerror(errno));
//             exit(1);
//         }
//         if (fstat(dfd, &sb) != 0 || !S_ISDIR(sb.st_mode)) {
//             errno = ENOTDIR;
//             fprintf(stderr, "%s: %d %s\n", DataPath, errno, strerror(errno));
//             exit(1);
//         }
//         if ((dp = opendir(DataPath))==NULL) {
//             perror("can't open dir");
//             exit(1);
//         }
//         printf("%-20s %s\n", "Directory:", DataPath);
//         while ((dirp = readdir(dp)) != NULL) {
//             if (fstatat(dfd, dirp->d_name, &sb, 0) == -1) {
//                 fprintf(stderr, "fstatat(\"%s\") failed (%d: %s)\n",
//                         dirp->d_name, errno, strerror(errno));
//                 exit(1);
//             }   
//             printf("%-20s %s\n", "File name:", dirp->d_name);
//         }
//         closedir(dp);
//         close(dfd);

