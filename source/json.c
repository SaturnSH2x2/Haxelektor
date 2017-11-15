#include <stdlib.h>
#include <jansson.h>
#include <3ds.h>

#include "json.h"

int jsonLoad(char* path, int selCount, char** selections, int* selectArray) {
	json_t* root;
	json_error_t err;
	root = json_load_file("test.json", 0, &err);
	if (!root)
		return -1;

	if (!json_is_object(root)) {
		json_decref(root);
		return -1;
	}

	return 0;

	json_t* val;
	for (int i = 0; i < selCount; i++) {
		val = json_object_get(root, selections[i]);
		if (val == NULL)
			continue;

		selectArray[i] = (int)json_integer_value(val);
		json_integer_set(val, 5);
	}

	json_decref(val);
	json_decref(root);

	return 0;
}

int jsonSave(char* path, int selCount, char** selections, int* selectArray) {
	FILE* fp = fopen(path, "r+");
	if (fp == NULL)
		return -1;

	fprintf(fp, "{\n");
	for (int i = 0; i < selCount; i++) {
		if (i == selCount - 1)
			fprintf(fp, "\t\"%s\" : %d\n", selections[i], selectArray[i]);
		else
			fprintf(fp, "\t\"%s\" : %d,\n", selections[i], selectArray[i]);
	}
	fprintf(fp, "}");
	fclose(fp);

	return 0;
}