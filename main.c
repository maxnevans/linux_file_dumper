#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/limits.h>
#include <dirent.h>


void printError(char *fullPath, const char* message, char* filename)
{
	fprintf(stderr, "%s:%s:%s\n", fullPath, message, filename);
}

int scanAndPrintDirectory(char *rootPath, int level, int *counter, DIR * descDir)
{
	const int PADDING = 4;
	int hasFiles = 0;

	// Skipping . and ..
	readdir(descDir);
	readdir(descDir);

	for (struct dirent* entity = readdir(descDir); entity != NULL; entity = readdir(descDir))
	{
		hasFiles++;
		(*counter)++;
		if (hasFiles == 1 && level) {
			printf("\n");
		}
		char fullPath[PATH_MAX];
		sprintf(fullPath, "%s/%s", rootPath, entity->d_name);
		struct stat entityDetails = {0};
		int retVal = stat(fullPath, &entityDetails);
		if (retVal == 0) {

			unsigned char dMode = (entityDetails.st_mode >> 15) & 7;
			unsigned char uMode = (entityDetails.st_mode >> 6) & 7;
			unsigned char gMode = (entityDetails.st_mode >> 3) & 7;
			unsigned char aMode = (entityDetails.st_mode >> 0) & 7;
			printf("%*sMode : %d%d%d | Type: %-4s | Size: %10ld bytes | Name: %s%s \n",
				level*PADDING, "",
				uMode, gMode, aMode,
				!dMode ? "dir" : "file",
				entityDetails.st_size,
				entity->d_name,
				!dMode ? "/" : ""				
			);

			if (!dMode)
			{
				DIR *ddNext = opendir(fullPath);
				if (ddNext == NULL) {

					printError(fullPath, "erroropendir" ,entity->d_name);

				}
				else {

					printf("%*sScanning inner directory %s: ", level*PADDING, "", fullPath);
					retVal = scanAndPrintDirectory(fullPath, level + 1, counter, ddNext);
					if (retVal == -1) {
						printf("Directory is empty!\n");
					}
					else if (retVal) {
						printf("\n");
						fprintf(stderr, "Error occured while scanning inner directory of %s!\n", fullPath);
					}

				}
			}
		}
		else {
			printError(rootPath, "errorgettingstat", entity->d_name);
		}
	}
	if (!hasFiles) return -1;

	return 0;
}


int main(int argc, char* argv[], char *envp[]){

	if (argc != 2) return fprintf(stderr, "Invalid amount of input arguments!\n");

	DIR *ddInitial = opendir(argv[1]);
	int countFilesWatched = 0;

	if (ddInitial){

		int retVal = scanAndPrintDirectory(argv[1], 0, &countFilesWatched, ddInitial);

		if (retVal) {
			fprintf(stderr, "Error occured while scanning inner directory of %s!\n", argv[1]);
		}

	}
	else {
		printError(argv[1], "erroropendir", argv[1]);
	}

	printf("Total entities watched: %d\n", countFilesWatched);


	return 0;
}