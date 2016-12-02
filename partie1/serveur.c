#include "serveur.h"

/**
	Liste les fichiers du répertoire path_to_dir dans la variable buffer
*/
void listdir(const char* path_to_dir, char* buffer){

	DIR* dir;
	dir = opendir(path_to_dir);
	if(dir == NULL) {
		perror("opendir ");
	}
	
	char path_file[512];
	int chaine_len = 0;
	struct dirent* entry;

	strcpy(buffer, "");

	while((entry = readdir(dir)) != NULL) {
		if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;

		strcpy(path_file, path_to_dir);
		strcat(path_file, "/");
		strcat(path_file, entry->d_name);

		strcat(buffer, lstattoa(path_file, entry->d_name));

		chaine_len = strlen(buffer);
	}

}

/**
	Retourne la chaine de caractères contenant les informations du fichier
	name dans le répertoire path_to_file
*/
char* lstattoa(char* path_to_file, char* name){
	
	struct stat file;
	char* file_infos = malloc(256 * sizeof(*file_infos));

	if(lstat(path_to_file, &file) < 0){
		perror("lstat ");
		return NULL;
	}

	// File type (d, - ou l)
	if(S_ISREG(file.st_mode)){
		strcpy(file_infos, "-");
	}
	if(S_ISDIR(file.st_mode)){
		strcpy(file_infos, "d");
	}
	if(S_ISLNK(file.st_mode)){
		strcpy(file_infos, "l");
	}
	
	// USR permissions tests
	if((file.st_mode & S_IRUSR) == S_IRUSR){
		strcpy(file_infos, strcat(file_infos, "r"));
	}
	else strcpy(file_infos, strcat(file_infos, "-"));

	if((file.st_mode & S_IWUSR) == S_IWUSR){
		strcpy(file_infos, strcat(file_infos, "w"));
	}
	else strcpy(file_infos, strcat(file_infos, "-"));

	if((file.st_mode & S_IXUSR) == S_IXUSR){
		strcpy(file_infos, strcat(file_infos, "x"));
	}
	else strcpy(file_infos, strcat(file_infos, "-"));

	// GRP permissions tests
	if((file.st_mode & S_IRGRP) == S_IRGRP){
		strcpy(file_infos, strcat(file_infos, "r"));
	}
	else strcpy(file_infos, strcat(file_infos, "-"));

	if((file.st_mode & S_IWGRP) == S_IWGRP){
		strcpy(file_infos, strcat(file_infos, "w"));
	}
	else strcpy(file_infos, strcat(file_infos, "-"));

	if((file.st_mode & S_IXGRP) == S_IXGRP){
		strcpy(file_infos, strcat(file_infos, "x"));
	}
	else strcpy(file_infos, strcat(file_infos, "-"));

	// OTH permissions tests
	if((file.st_mode & S_IROTH) == S_IROTH){
		strcpy(file_infos, strcat(file_infos, "r"));
	}
	else strcpy(file_infos, strcat(file_infos, "-"));

	if((file.st_mode & S_IWOTH) == S_IWOTH){
		strcpy(file_infos, strcat(file_infos, "w"));
	}
	else strcpy(file_infos, strcat(file_infos, "-"));

	if((file.st_mode & S_IXOTH) == S_IXOTH){
		strcpy(file_infos, strcat(file_infos, "x"));
	}
	else strcpy(file_infos, strcat(file_infos, "-"));

	// add space
	strcpy(file_infos, strcat(file_infos, " "));

	char file_size [10];
	int ret_sprintf = 0;

	ret_sprintf = sprintf(file_size, "%d", (int)file.st_size);
	if(ret_sprintf == -1) perror("sprintf ");
	else strcpy(file_infos, strcat(file_infos, file_size));

	 // add space
	strcpy(file_infos, strcat(file_infos, "\t"));

	// add name
	strcpy(file_infos, strcat(file_infos, name));

	// add name
	strcpy(file_infos, strcat(file_infos, "\n"));
	
	return file_infos;
}